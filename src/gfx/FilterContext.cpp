#include "FilterContext.h"
#include "ModelBank.h"
#include "TestParams.h"
FilterContext::FilterContext() {

}

FilterContext::~FilterContext() {

}

void FilterContext::Init(DX12Context* context) {
	//create resources
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(g_TestParams.BatchSize * 3 * g_TestParams.BatchCount * sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	CD3DX12_RESOURCE_DESC drawBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(g_TestParams.BatchCount * sizeof(IndirectDrawCall), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_IndexBuffers[i]));
		context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&drawBufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_DrawArgsBuffers[i]));
	}
	//counter resource
	CD3DX12_RESOURCE_DESC counterBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * MAX_SIMUL_PASSES * 64, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&counterBufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_CounterBuffer));
	//Descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 3;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_FilterDescriptorHeaps[i]));
	}
	m_DescHeapIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//fill descriptorheaps
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_FilterDescriptorHeaps[i]->GetCPUDescriptorHandleForHeapStart());
		//draw args
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = g_TestParams.BatchCount;
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		context->Device->CreateUnorderedAccessView(m_DrawArgsBuffers[i].Get(), nullptr, &uavDesc, cpuHandle);
		//indices output
		uavDesc.Buffer.NumElements = g_TestParams.BatchCount * g_TestParams.BatchSize * 3;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT);
		context->Device->CreateUnorderedAccessView(m_IndexBuffers[i].Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
		//counter buffer
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = i * 64; //make the first element i offset in to make the passes alternate between two indices
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = 64;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		context->Device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	}

	//index buffer views
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		m_IndexBufferViews[i].BufferLocation = m_IndexBuffers[i]->GetGPUVirtualAddress();
		m_IndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		m_IndexBufferViews[i].SizeInBytes = indexBufferDesc.Width;
	}

}

void FilterContext::Clear() {
	m_CurrentBatchCount = 0;
	m_CurrentDraw = 0;
	m_CurrentBatch = 0;
}

void FilterContext::BeginFilter(ID3D12GraphicsCommandList* cmdList) {
	m_CurrentFilterIndex = ++m_CurrentFilterIndex % MAX_SIMUL_PASSES;

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	g_BufferManager.SwitchState("IndirectBuffer", D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void FilterContext::BeginRender(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut) {
	m_CurrentRenderIndex = ++m_CurrentRenderIndex % MAX_SIMUL_PASSES;

	*drawArgsOut = m_DrawArgsBuffers[m_CurrentRenderIndex].Get();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_DrawArgsBuffers[m_CurrentRenderIndex].Get()));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	g_BufferManager.SwitchState("IndirectBuffer", D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	cmdList->IASetIndexBuffer(&m_IndexBufferViews[m_CurrentRenderIndex]);
}

// returns wether we are full of geometry or not, batchCountOut returns the number of batches that should be disbatched
bool FilterContext::AddBatches(UINT batchCount, UINT& batchCountOut) {

	//if we have some remainder left of the last batch use that now
	if (m_BatchRemainder > 0) {
		batchCount = m_BatchRemainder;
		m_CurrentBatchCount = 0;
	}

	if (m_CurrentBatchCount + batchCount > g_TestParams.BatchCount) {
		m_BatchRemainder = (m_CurrentBatchCount + batchCount) - g_TestParams.BatchCount;
		batchCountOut = batchCount - m_BatchRemainder;
		m_CurrentBatch += batchCountOut;
		m_CurrentBatchCount += batchCountOut;
		return true;
	} else {
		m_CurrentBatchCount += batchCount;
		batchCountOut = batchCount;
		m_CurrentBatch += batchCount;
		m_BatchRemainder = 0;
		return false;
	}
}