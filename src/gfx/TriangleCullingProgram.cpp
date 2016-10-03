#include "TriangleCullingProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "BufferManager.h"

TriangleCullingProgram::TriangleCullingProgram() {

}

TriangleCullingProgram::~TriangleCullingProgram() {

}

void TriangleCullingProgram::Init(DX12Context* context) {

	m_Shader.LoadFromFile(L"src/shaders/Computeculling.hlsl", COMPUTE_SHADER_BIT, &context->Extensions);
	//root signature
	RootSignatureFactory rootFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE range;
	for (int i = 0; i < ROOT_PARAM_COUNT; ++i) {
		switch (i) {
		case INPUT_COUNT_C:
			rootFact.AddConstant(1, 0);
			break;
		case INPUT_DESC:
			range.BaseShaderRegister = 0;
			range.NumDescriptors = 1;
			range.OffsetInDescriptorsFromTableStart = 0;
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.RegisterSpace = 0;
			ranges.push_back(range);

			range.BaseShaderRegister = 0;
			range.NumDescriptors = 2;
			range.OffsetInDescriptorsFromTableStart = 1;
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range.RegisterSpace = 0;
			ranges.push_back(range);
			rootFact.AddDescriptorTable(ranges);
			break;
		default:
			break;
		}
	}
	rootFact.AddExtensions(&context->Extensions);
	m_RootSignature = rootFact.CreateSignture(context->Device.Get());

	PipelineStateFactory pipeFact;
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	pipeFact.SetRootSignature(m_RootSignature.Get());
	m_PipelineState = pipeFact.CreateComputeState(context);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 3;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_GPUDescriptorHeap));
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CPUDescriptorHeap));

	//D3D12_RESOURCE_DESC resourceDesc = {};
	//resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	//resourceDesc.Alignment = 0;
	//resourceDesc.Height = 1;
	//resourceDesc.DepthOrArraySize = 1;
	//resourceDesc.MipLevels = 1;
	//resourceDesc.SampleDesc.Count = 1;
	//resourceDesc.SampleDesc.Quality = 0;
	//resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	//resourceDesc.Width = sizeof(IndirectDrawCall) * 10000;

	//context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	//	&resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_OutputBuffer));

	//CD3DX12_RESOURCE_DESC counterDesc = CD3DX12_RESOURCE_DESC::Buffer(4096, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	//	&counterDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_CounterBuffer));

	g_BufferManager.CreateStructuredBuffer("CulledIndirectBuffer", nullptr, sizeof(IndirectDrawCall) * 10000, sizeof(IndirectDrawCall));
	g_BufferManager.CreateStructuredBuffer("CullingCounterBuffer", nullptr, 4096, sizeof(UINT));

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE gpuHandle(m_GPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	m_DescIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = 10000;
	srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	context->Device->CreateShaderResourceView(g_BufferManager.GetBufferResource("IndirectBuffer"), &srvDesc, cpuHandle);
	context->Device->CreateShaderResourceView(g_BufferManager.GetBufferResource("IndirectBuffer"), &srvDesc, gpuHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = 10000;
	uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	context->Device->CreateUnorderedAccessView(g_BufferManager.GetBufferResource("CulledIndirectBuffer"), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescIncSize));
	context->Device->CreateUnorderedAccessView(g_BufferManager.GetBufferResource("CulledIndirectBuffer"), nullptr, &uavDesc, gpuHandle.Offset(1, m_DescIncSize));

	uavDesc = {};
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = 1024;
	uavDesc.Buffer.StructureByteStride = sizeof(UINT);
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	context->Device->CreateUnorderedAccessView(g_BufferManager.GetBufferResource("CullingCounterBuffer"), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescIncSize));
	context->Device->CreateUnorderedAccessView(g_BufferManager.GetBufferResource("CullingCounterBuffer"), nullptr, &uavDesc, gpuHandle.Offset(1, m_DescIncSize));

	m_Context = context;

#ifdef _DEBUG
	//m_CounterBuffer->SetName(L"CounterBuffer");
	//m_OutputBuffer->SetName(L"CulledIndirectBuffer");
#endif
}

void TriangleCullingProgram::Disbatch(RenderQueue* queue) {
	g_BufferManager.SwitchState("IndirectBuffer", D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	g_BufferManager.SwitchState("CulledIndirectBuffer", D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	g_BufferManager.SwitchState("CullingCounterBuffer", D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//clear uav counter
	UINT vals[4] = { 0,0,0,0 };
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CPUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 2, m_DescIncSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_GPUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 2, m_DescIncSize);

	ID3D12GraphicsCommandList* cmdList = m_Context->CommandList.Get();

	cmdList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, g_BufferManager.GetBufferResource("CullingCounterBuffer"), vals, 0, nullptr);
	
	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->SetComputeRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = { m_GPUDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetComputeRoot32BitConstant(INPUT_COUNT_C, queue->GetDrawCount(), 0);
	cmdList->SetComputeRootDescriptorTable(INPUT_DESC, m_GPUDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	
	const int workGroups = (queue->GetDrawCount() + 64 - 1) / 64;

	cmdList->Dispatch(workGroups, 1, 1);
}