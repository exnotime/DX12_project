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

void BufferManager::Init(ID3D12Device* device, UINT maxBuffers) {
	D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc = {};
	bufferHeapDesc.NumDescriptors = maxBuffers;
	bufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(device->CreateDescriptorHeap(&bufferHeapDesc, IID_PPV_ARGS(&m_DescriptorHeap)), L"Error creating descriptor heap for buffer manager");
	
	m_HeapIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_Device = device;
}

void BufferManager::CreateConstBuffer(const std::string& name, UINT size) {
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
	HR(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->Resource[0])),
		L"Error creating constant buffer");
	buffer->Offset = m_BufferCounter++;
	buffer->Type = CONST_BUFFER;

	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
	viewDesc.BufferLocation = buffer->Resource[0]->GetGPUVirtualAddress();
	viewDesc.SizeInBytes = realSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), buffer->Offset * m_HeapIncrementSize);
	m_Device->CreateConstantBufferView(&viewDesc, handle);

#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource[0]->SetName(wName);
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::CreateIndirectBuffer(const std::string& name, UINT size) {
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
	HR(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->Resource[0])),
		L"Error creating indirect buffer");

	HR(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->UploadHeap)),
		L"Error creating indirect buffer upload heap");
	buffer->Offset = m_BufferCounter++;
	buffer->Type = INDIRECT_BUFFER;
	buffer->State = D3D12_RESOURCE_STATE_COPY_DEST;
#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource[0]->SetName(wName);
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::CreateStructuredBuffer(const std::string& name, UINT size, UINT structureSize) {
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
	HR(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->Resource[0])),
		L"Error creating structured buffer");
	HR(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(realSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer->UploadHeap)),
		L"Error creating structured buffer upload heap");

	buffer->Offset = m_BufferCounter++;
	buffer->Type = STRUCTURED_BUFFER;
	buffer->State = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.NumElements = size / structureSize;
	viewDesc.Buffer.StructureByteStride = structureSize;
	viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), buffer->Offset * m_HeapIncrementSize);

	m_Device->CreateShaderResourceView( buffer->Resource[0].Get(), &viewDesc, handle);
#ifdef _DEBUG
	wchar_t* wName = convertCharArrayToLPCWSTR(name.c_str());
	buffer->Resource[0]->SetName(wName);
	delete[] wName;
#endif

	m_Buffers.emplace(name, buffer);
}

void BufferManager::UpdateBuffer(ID3D12GraphicsCommandList* cmdList, const std::string& name, void* data, UINT size) {
	if (size == 0) {
		return;
	}
	if (data == nullptr) {
		return;
	}
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end() && !buffer->second->GPUOnly) {

		switch (buffer->second->Type)
		{
		case CONST_BUFFER: {
			void* gpuPtr;
			buffer->second->Resource[0]->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->Resource[0]->Unmap(0, nullptr);
			break;
		}
		case STRUCTURED_BUFFER: {
			SwitchState(cmdList,name, D3D12_RESOURCE_STATE_COPY_DEST);
			void* gpuPtr;
			buffer->second->UploadHeap->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->UploadHeap->Unmap(0, nullptr);
			cmdList->CopyBufferRegion(buffer->second->Resource[0].Get(), 0, buffer->second->UploadHeap.Get(), 0, size);
			buffer->second->State = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		}
		case INDIRECT_BUFFER: {
			SwitchState(cmdList, name, D3D12_RESOURCE_STATE_COPY_DEST);
			void* gpuPtr;
			buffer->second->UploadHeap->Map(0, nullptr, &gpuPtr);
			memcpy(gpuPtr, data, size);
			buffer->second->UploadHeap->Unmap(0, nullptr);
			cmdList->CopyBufferRegion(buffer->second->Resource[0].Get(), 0, buffer->second->UploadHeap.Get(), 0, size);
			buffer->second->State = D3D12_RESOURCE_STATE_COPY_DEST;
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
		buffer->second->Resource[0]->Map(0, nullptr, &data);
		return data;
	}
	return nullptr;
}

void BufferManager::UnMapBuffer(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		buffer->second->Resource[0]->Unmap(0, nullptr);
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
		return buffer->second->Resource[0]->GetGPUVirtualAddress();
	}
	return D3D12_GPU_VIRTUAL_ADDRESS();
}

ID3D12Resource* BufferManager::GetBufferResource(const std::string& name) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		return buffer->second->Resource[0].Get();
	}
	return nullptr;
}

void BufferManager::SwitchState(ID3D12GraphicsCommandList* cmdList, const std::string& name, D3D12_RESOURCE_STATES state) {
	auto& buffer = m_Buffers.find(name);
	if (buffer != m_Buffers.end()) {
		if (state == buffer->second->State)
			return;
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			buffer->second->Resource[0].Get(), buffer->second->State, state));
		buffer->second->State = state;
	}
}