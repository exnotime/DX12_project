#include "DrawCullingProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "BufferManager.h"
#include "FilterContext.h"
#include "TestParams.h"
DrawCullingProgram::DrawCullingProgram() {

}

DrawCullingProgram::~DrawCullingProgram() {

}

void DrawCullingProgram::Init(DX12Context* context, FilterContext* filterContext) {
	m_WaveSize = context->Extensions.WaveSize;
	m_Shader.LoadFromFile(L"src/shaders/ComputeCulling.hlsl", COMPUTE_SHADER_BIT, &context->Extensions);
	//root signature
	RootSignatureFactory rootFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE range;
	for (int i = 0; i < ROOT_PARAM_COUNT; ++i) {
		switch (i) {
		case INPUT_COUNT_C:
			rootFact.AddConstant(2, 0);
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
		}
	}
	rootFact.AddExtensions(&context->Extensions);
	m_RootSignature = rootFact.CreateSignture(context->Device.Get());
	PipelineStateFactory pipeFact;
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	pipeFact.SetRootSignature(m_RootSignature.Get());
	m_PipelineState = pipeFact.CreateComputeState(context);
	//resources
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndirectDrawCall) * 8192, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < MAX_SIMUL_PASSES; ++i) {
		context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_OutputBuffer[i]));
	}
	bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * MAX_SIMUL_PASSES * 4096, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_CounterBuffer));

	//descriptors
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = DESC_HEAP_SIZE * MAX_SIMUL_PASSES;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeap));

	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CounterClearHeaps[0]));

	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CounterClearHeaps[1]));

	m_HeapDescIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (int i = 0; i < MAX_SIMUL_PASSES; ++i) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart(), DESC_HEAP_SIZE * i * m_HeapDescIncSize);
		//input
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = g_TestParams.CurrentTest.BatchCount;
		srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		context->Device->CreateShaderResourceView(filterContext->GetDrawArgsResource(i), &srvDesc, handle);
		//output
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = g_TestParams.CurrentTest.BatchCount;
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		context->Device->CreateUnorderedAccessView(m_OutputBuffer[i].Get(), nullptr, &uavDesc, handle.Offset(1, m_HeapDescIncSize));
		//counter
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = 256 * MAX_SIMUL_PASSES;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		context->Device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, handle.Offset(1, m_HeapDescIncSize));
		//create counter uavs for clearing as well
		context->Device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, m_CounterClearHeaps[i]->GetCPUDescriptorHandleForHeapStart());
	}
}

void DrawCullingProgram::Reset(DX12Context* context, FilterContext* filterContext) {
	for (int i = 0; i < MAX_SIMUL_PASSES; ++i) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart(), DESC_HEAP_SIZE * i * m_HeapDescIncSize);
		//input
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = g_TestParams.CurrentTest.BatchCount;
		srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		context->Device->CreateShaderResourceView(filterContext->GetDrawArgsResource(i), &srvDesc, handle);
		//output
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = g_TestParams.CurrentTest.BatchCount;
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		context->Device->CreateUnorderedAccessView(m_OutputBuffer[i].Get(), nullptr, &uavDesc, handle.Offset(1, m_HeapDescIncSize));
	}
}

void DrawCullingProgram::ClearCounters(ID3D12GraphicsCommandList* cmdList) {
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	UINT vals[4] = { 0,0,0,0 };
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CounterClearHeaps[1]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_CounterClearHeaps[0]->GetGPUDescriptorHandleForHeapStart());

	cmdList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, m_CounterBuffer.Get(), vals, 0, nullptr);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
}

void DrawCullingProgram::Disbatch(ID3D12GraphicsCommandList* cmdList, FilterContext* filterContext) {

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(filterContext->GetDrawArgsResource(filterContext->GetFilterIndex()),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_OutputBuffer[filterContext->GetFilterIndex()].Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->SetComputeRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetComputeRootDescriptorTable(INPUT_DESC, CD3DX12_GPU_DESCRIPTOR_HANDLE(
		m_DescHeap->GetGPUDescriptorHandleForHeapStart(), m_HeapDescIncSize * filterContext->GetFilterIndex() * DESC_HEAP_SIZE));

	cmdList->SetComputeRoot32BitConstant(INPUT_COUNT_C, filterContext->GetBatchCount(), 0);
	cmdList->SetComputeRoot32BitConstant(INPUT_COUNT_C, filterContext->GetCounterOffset(), 1);

	UINT disbatchCount = (filterContext->GetBatchCount() + m_WaveSize - 1) / m_WaveSize;
	cmdList->Dispatch(disbatchCount, 1, 1);
}