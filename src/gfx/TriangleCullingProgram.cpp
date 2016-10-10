#include "TriangleCullingProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "BufferManager.h"
#include "ModelBank.h"
TriangleCullingProgram::TriangleCullingProgram() {

}

TriangleCullingProgram::~TriangleCullingProgram() {
	if (m_DrawListArray) delete[] m_DrawListArray;
}

void TriangleCullingProgram::Init(DX12Context* context, const UINT maxTriangleCount, const UINT batchSize) {
	m_Context = context;
	m_Shader.LoadFromFile(L"src/shaders/TriangleCulling.hlsl", COMPUTE_SHADER_BIT, &context->Extensions);
	//Root sign
	RootSignatureFactory rootSignFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	for (int i = 0; i < ROOT_PARAM_COUNT; ++i) {
		switch (i)
		{
		case PER_FRAME_CB:
			rootSignFact.AddConstantBufferView(0);
			break;
		case CONSTANTS_C:
			rootSignFact.AddConstant(2, 1);
			break;
		case INPUT_DT:
			ranges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0));
			ranges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 0, 4));
			rootSignFact.AddDescriptorTable(ranges);
			break;
		}
	}
	rootSignFact.AddExtensions(&context->Extensions);
	m_RootSign = rootSignFact.CreateSignture(context->Device.Get());
	//Pipe state
	PipelineStateFactory pipeFact;
	pipeFact.SetRootSignature(m_RootSign.Get());
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	m_PipeState = pipeFact.CreateComputeState(context);
	//Resources
	//index buffer
	m_MaxTrianglecount = maxTriangleCount;
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_MaxTrianglecount * 3 * sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_CulledIndexBuffer));
	//draw args buffer
	m_BatchSize = batchSize;
	m_MaxBatchCount = (m_MaxTrianglecount + m_BatchSize - 1) / m_BatchSize;

	bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_MaxBatchCount * sizeof(IndirectDrawCall), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_CulledDrawArgsBuffer));

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_SplitDrawArgsBuffer));

	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_SplitDrawArgsBufferUpload));

	//Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 6;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeap));
	m_DescHeapIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_IBOView.BufferLocation = m_CulledIndexBuffer->GetGPUVirtualAddress();
	m_IBOView.Format = DXGI_FORMAT_R32_UINT;
	m_IBOView.SizeInBytes = m_MaxTrianglecount * 3 * sizeof(UINT);

	m_DrawListArray = new IndirectDrawCall[m_MaxBatchCount];
}

void TriangleCullingProgram::CreateDescriptorTable() {
	ID3D12Device* device =  m_Context->Device.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//draw args input
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_MaxBatchCount;
	srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	device->CreateShaderResourceView(m_SplitDrawArgsBuffer.Get(), &srvDesc, cpuHandle);
	//vertices
	srvDesc.Buffer.NumElements = g_ModelBank.GetVertexCount();
	srvDesc.Buffer.StructureByteStride = sizeof(glm::vec3);
	device->CreateShaderResourceView(g_ModelBank.GetVertexBufferResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//indices
	srvDesc.Buffer.NumElements = g_ModelBank.GetIndexCount();
	srvDesc.Buffer.StructureByteStride = sizeof(UINT);
	device->CreateShaderResourceView(g_ModelBank.GetIndexBufferResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//shader inputs
	srvDesc.Buffer.NumElements = 10000;
	srvDesc.Buffer.StructureByteStride = sizeof(ShaderInput);
	device->CreateShaderResourceView(g_BufferManager.GetBufferResource("ShaderInputBuffer"), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//draw args output
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_MaxBatchCount;
	uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(m_CulledDrawArgsBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//indices output
	uavDesc.Buffer.NumElements = m_MaxTrianglecount * 3;
	uavDesc.Buffer.StructureByteStride = sizeof(UINT);
	device->CreateUnorderedAccessView(m_CulledIndexBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
}

void TriangleCullingProgram::Disbatch(RenderQueue* queue) {
	m_BatchCount = SplitMeshes(queue);
	ID3D12GraphicsCommandList* cmdList = m_Context->CommandList.Get();

	ID3D12DescriptorHeap* heaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->SetComputeRootSignature(m_RootSign.Get());
	cmdList->SetPipelineState(m_PipeState.Get());
	cmdList->SetComputeRootConstantBufferView(PER_FRAME_CB, g_BufferManager.GetGPUHandle("cbPerFrame"));
	cmdList->SetComputeRootDescriptorTable(INPUT_DT, m_DescHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledIndexBuffer.Get(),
		D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledDrawArgsBuffer.Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	for (int i = 0; i < m_BatchCount; ++i) {
		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, i, 0);
		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, i * m_BatchSize * 3, 1);
		//const UINT disbatchSize = (m_BatchSize + 256 - 1) / 256;
		cmdList->Dispatch(1, 1, 1); //look into multiple work groups
	}

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledIndexBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledDrawArgsBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
}

UINT TriangleCullingProgram::SplitMeshes(RenderQueue* queue) {
	IndirectDrawCall draw;
	UINT batchCounter = 0;
	for (auto& draw : queue->GetDrawList()) {
		UINT batchCount = ((draw.DrawArgs.IndexCountPerInstance / 3) + m_BatchSize - 1) / m_BatchSize;
		UINT indexCount = draw.DrawArgs.IndexCountPerInstance;
		for (int i = 0; i < batchCount; ++i) {
			m_DrawListArray[batchCounter + i] = draw;
			m_DrawListArray[batchCounter + i].DrawArgs.StartIndexLocation = draw.DrawArgs.StartIndexLocation + m_BatchSize * 3 * i; // (batchCounter + i) * m_BatchSize * 3;
			m_DrawListArray[batchCounter + i].DrawArgs.IndexCountPerInstance = (indexCount > m_BatchSize * 3) ? m_BatchSize * 3 : indexCount;
			indexCount -= m_BatchSize * 3;
		}
		batchCounter += batchCount;
	}
	//transfer to GPU
	void* data;
	D3D12_RANGE range = { 0,0 };
	m_SplitDrawArgsBufferUpload->Map(0, &range, &data);
	memcpy(data, m_DrawListArray, sizeof(IndirectDrawCall) * batchCounter);
	range.End = sizeof(IndirectDrawCall) * batchCounter;
	m_SplitDrawArgsBufferUpload->Unmap(0, &range);
	ID3D12GraphicsCommandList* cmdList = m_Context->CommandList.Get();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SplitDrawArgsBuffer.Get(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->CopyBufferRegion(m_SplitDrawArgsBuffer.Get(), 0, m_SplitDrawArgsBufferUpload.Get(), 0, sizeof(IndirectDrawCall) * batchCounter);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SplitDrawArgsBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	return batchCounter;
}