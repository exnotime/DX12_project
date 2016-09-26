#include "BufferManager.h"

BufferManager::BufferManager() {

}

BufferManager& BufferManager::GetInstance() {
	static BufferManager instance;
	return instance;
}

BufferManager::~BufferManager() {
	for (auto& buffer : m_Buffers) {
		delete buffer.second;
	}
}

void BufferManager::Init(DX12Context* context, UINT maxBuffers) {
	m_Context = context;
	D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {};
	bufferHeapDesc.NumDescriptors = maxBuffers;
	bufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(context->Device->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&m_DescriptorHeap)), L"Error creating descriptor heap for buffer manager");
	
	m_HeapIncrementSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void BufferManager::CreateConstBuffer(const std::string& name, void* data, UINT size) {
	if (size == 0) {
		return;
	}
	auto& buf = m_Buffers.find(name);
	if (buf != m_Buffers.end()) {
		//buffer already exist
		return;
	}
	UINT realSize = (size + 255) & ~255;
	Buffer* buffer = new Buffer();
	HR(m_Context->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->Resource)),
		L"Error creating constant buffer");
	buffer->Offset = m_BufferCounter++;
	buffer->Type = CONST_BUFFER;

	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
	viewDesc.BufferLocation = buffer->Resource->GetGPUVirtualAddress();
	viewDesc.SizeInBytes = realSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), buffer->Offset * m_HeapIncrementSize);
	m_Context->Device->CreateConstantBufferView(&viewDesc, handle);

	if (data != nullptr) {
		//transfer data
		void* gpuPtr;
		buffer->Resource->Map(0, nullptr, &gpuPtr);
		memcpy(gpuPtr, data, realSize);
		buffer->Resource->Unmap(0, nullptr);
	}
#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource->SetName(wName);
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::CreateIndirectBuffer(const std::string& name, void* data, UINT size) {
	if (size == 0) {
		return;
	}
	auto& buf = m_Buffers.find(name);
	if (buf != m_Buffers.end()) {
		//buffer already exist
		return;
	}
	UINT realSize = (size + 255) & ~255;
	Buffer* buffer = new Buffer();
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(realSize);
	HR(m_Context->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->Resource)),
		L"Error creating indirect buffer");

	HR(m_Context->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->UploadHeap)),
		L"Error creating indirect buffer upload heap");
	buffer->Offset = m_BufferCounter++;
	buffer->Type = INDIRECT_BUFFER;

	if (data != nullptr) {
		D3D12_SUBRESOURCE_DATA sub_data = {};
		sub_data.pData = data;
		sub_data.RowPitch = realSize;
		sub_data.SlicePitch = sub_data.RowPitch;

		UpdateSubresources(m_Context->CommandList.Get(), buffer->Resource.Get(), buffer->UploadHeap.Get(), 0, 0, 1, &sub_data);

		m_Context->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(buffer->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
	}
#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource->SetName(wName);
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::CreateStructuredBuffer(const std::string& name, void* data, UINT size, UINT structureSize) {
	if (size == 0) {
		return;
	}
	auto& buf = m_Buffers.find(name);
	if (buf != m_Buffers.end()) {
		//buffer already exist
		return;
	}
	UINT realSize = (size + 255) & ~255;
	Buffer* buffer = new Buffer();
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(realSize);
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	HR(m_Context->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->Resource)),
		L"Error creating structured buffer");

	HR(m_Context->Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->UploadHeap)),
		L"Error creating structured buffer upload heap");

	buffer->Offset = m_BufferCounter++;
	buffer->Type = STRUCTURED_BUFFER;

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.NumElements = size / structureSize;
	viewDesc.Buffer.StructureByteStride = structureSize;
	viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), buffer->Offset * m_HeapIncrementSize);
	m_Context->Device->CreateShaderResourceView( buffer->Resource.Get(), &viewDesc, handle);

	if (data != nullptr) {
		D3D12_SUBRESOURCE_DATA sub_data = {};
		sub_data.pData = data;
		sub_data.RowPitch = realSize;
		sub_data.SlicePitch = sub_data.RowPitch;

		UpdateSubresources(m_Context->CommandList.Get(), buffer->Resource.Get(), buffer->UploadHeap.Get(), 0, 0, 1, &sub_data);

		m_Context->CommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(buffer->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}
#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource->SetName(wName);
	buffer->UploadHeap->SetName(L"Structured buffer upload heap");
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::UpdateBuffer(const std::string& name, void* data, UINT size) {
	if (size == 0) {
		return;
	}
	if (data == nullptr) {
		return;
	}
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {

		switch (buffer->second->Type)
		{
		case CONST_BUFFER: {
			void* gpuPtr;
			buffer->second->Resource->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->Resource->Unmap(0, nullptr);
			break;
		}
		case STRUCTURED_BUFFER: {
			m_Context->CopyCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(buffer->second->Resource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
			void* gpuPtr;
			buffer->second->UploadHeap->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->UploadHeap->Unmap(0, nullptr);
			m_Context->CopyCommandList->CopyBufferRegion(buffer->second->Resource.Get(), 0, buffer->second->UploadHeap.Get(), 0, size);
			//m_Context->CommandList->ResourceBarrier(1,
			//	&CD3DX12_RESOURCE_BARRIER::Transition(buffer->second->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
			break;
		}
		case INDIRECT_BUFFER: {
			m_Context->CopyCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(buffer->second->Resource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
			void* gpuPtr;
			buffer->second->UploadHeap->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->UploadHeap->Unmap(0, nullptr);
			m_Context->CopyCommandList->CopyBufferRegion(buffer->second->Resource.Get(), 0, buffer->second->UploadHeap.Get(), 0, size);
			//m_Context->CommandList->ResourceBarrier(1,
			//	&CD3DX12_RESOURCE_BARRIER::Transition(buffer->second->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
			break;
		}
		default:
			break;
		};
	}
}

void* BufferManager::MapBuffer(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		void* data;
		buffer->second->Resource->Map(0, nullptr, &data);
		return data;
	}
	return nullptr;
}

void BufferManager::UnMapBuffer(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		buffer->second->Resource->Unmap(0, nullptr);
	}
}

CD3DX12_CPU_DESCRIPTOR_HANDLE BufferManager::GetCPUHandle(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), buffer->second->Offset * m_HeapIncrementSize);
		return handle;
	}
	return CD3DX12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetGPUHandle(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		return buffer->second->Resource->GetGPUVirtualAddress();
	}
	return D3D12_GPU_VIRTUAL_ADDRESS();
}

ID3D12Resource* BufferManager::GetBufferResource(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		return buffer->second->Resource.Get();
	}
	return nullptr;
}