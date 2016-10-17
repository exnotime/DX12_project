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
			ranges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0));
			ranges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0, 0, 5));
			rootSignFact.AddDescriptorTable(ranges);
			break;
		}
	}
	rootSignFact.AddExtensions(&context->Extensions);

	D3D12_STATIC_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampDesc.MipLODBias = 0;
	sampDesc.MaxAnisotropy = 1.0f;
	sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampDesc.MinLOD = 0.0f;
	sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	sampDesc.ShaderRegister = 0;
	sampDesc.RegisterSpace = 0;
	sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootSignFact.AddStaticSampler(sampDesc);
	m_RootSign = rootSignFact.CreateSignture(context->Device.Get());
	//Pipe state
	PipelineStateFactory pipeFact;
	pipeFact.SetRootSignature(m_RootSign.Get());
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	m_PipeState = pipeFact.CreateComputeState(context);
	//Resources
	//index buffer
	m_MaxTriangleCount = maxTriangleCount;
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_MaxTriangleCount * 3 * sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_CulledIndexBuffer));
	//draw args buffer
	m_BatchSize = batchSize;
	m_MaxBatchCount = (m_MaxTriangleCount + m_BatchSize - 1) / m_BatchSize;

	bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_MaxBatchCount * sizeof(IndirectDrawCall), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_CulledDrawArgsBuffer));

	//Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 8;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeap));
	m_DescHeapIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_IBOView.BufferLocation = m_CulledIndexBuffer->GetGPUVirtualAddress();
	m_IBOView.Format = DXGI_FORMAT_R32_UINT;
	m_IBOView.SizeInBytes = m_MaxTriangleCount * 3 * sizeof(UINT);

	m_DrawListArray = new IndirectDrawCall[m_MaxBatchCount];
}

void TriangleCullingProgram::CreateDescriptorTable(HiZProgram* hizProgram) {
	ID3D12Device* device =  m_Context->Device.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//draw args input
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = 10000;
	srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	device->CreateShaderResourceView(g_BufferManager.GetBufferResource("IndirectBuffer"), &srvDesc, cpuHandle);
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
	//hi-z
	srvDesc.Texture2D.MipLevels = hizProgram->GetMipCount();
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	device->CreateShaderResourceView(hizProgram->GetResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
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
	uavDesc.Buffer.NumElements = m_MaxTriangleCount * 3;
	uavDesc.Buffer.StructureByteStride = sizeof(UINT);
	device->CreateUnorderedAccessView(m_CulledIndexBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//instrumentation buffer
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	uavDesc.Buffer.NumElements = 32;
	uavDesc.Buffer.StructureByteStride = 0;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(g_BufferManager.GetBufferResource("CullingCounterBuffer"), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));

}

void TriangleCullingProgram::Disbatch(RenderQueue* queue) {
	//m_BatchCount = SplitMeshes(queue);
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
	g_BufferManager.SwitchState("IndirectBuffer", D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	m_BatchCount = 0;
	int i = 0;
	for (auto& draw : queue->GetDrawList()) {
		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, m_BatchCount, 0);
		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, i++, 1);
		UINT batchCount = ((draw.DrawArgs.IndexCountPerInstance / 3) + m_BatchSize - 1) / m_BatchSize;
		cmdList->Dispatch(batchCount, 1, 1);
		m_BatchCount += batchCount;
	}
	g_BufferManager.SwitchState("IndirectBuffer", D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledIndexBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CulledDrawArgsBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

UINT TriangleCullingProgram::SplitMeshes(RenderQueue* queue) {
	IndirectDrawCall draw;
	UINT batchCounter = 0;
	for (auto& draw : queue->GetDrawList()) {
		UINT batchCount = ((draw.DrawArgs.IndexCountPerInstance / 3) + m_BatchSize - 1) / m_BatchSize;
		UINT indexCount = draw.DrawArgs.IndexCountPerInstance;
		for (int i = 0; i < batchCount; ++i) {
			m_DrawListArray[batchCounter + i] = draw;
			m_DrawListArray[batchCounter + i].DrawArgs.StartIndexLocation = draw.DrawArgs.StartIndexLocation + m_BatchSize * 3 * i;
			m_DrawListArray[batchCounter + i].DrawArgs.IndexCountPerInstance = (indexCount > m_BatchSize * 3) ? m_BatchSize * 3 : indexCount;
			indexCount -= m_BatchSize * 3;
		}
		batchCounter += batchCount;
	}

	return batchCounter;
}