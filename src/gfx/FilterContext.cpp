#include "FilterContext.h"
#include "ModelBank.h"
#include "TestParams.h"
FilterContext::FilterContext() {

}

FilterContext::~FilterContext() {

}

void FilterContext::Init(ID3D12Device* device) {
	//create resources
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BATCH_SIZE * TRI_COUNT * 3 * BATCH_COUNT * sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	CD3DX12_RESOURCE_DESC drawBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BATCH_COUNT * sizeof(IndirectDrawCall), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_IndexBuffers[i]));
		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&drawBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_DrawArgsBuffers[i]));
	}
	//counter resource
	CD3DX12_RESOURCE_DESC counterBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * (64 + BATCH_COUNT * 2), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&counterBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_CounterBuffer));
	//resource to copy the counters into
	CD3DX12_RESOURCE_DESC copyBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * 64);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
		&copyBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_CopyBuffer));
	//Descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 4;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_FilterDescriptorHeaps[i]));
	}
	// To clear the counter we need to have 2 descriptor heaps for the uav
	// one cpu visible and one shader visible
	heapDesc.NumDescriptors = 5;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CounterClearHeaps[0]));
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CounterClearHeaps[1]));

	m_DescHeapIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//fill descriptorheaps
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_FilterDescriptorHeaps[i]->GetCPUDescriptorHandleForHeapStart());
		//draw args
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = BATCH_COUNT;
		uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		device->CreateUnorderedAccessView(m_DrawArgsBuffers[i].Get(), nullptr, &uavDesc, cpuHandle);
		//indices output
		uavDesc.Buffer.NumElements = BATCH_COUNT * BATCH_SIZE * TRI_COUNT * 3;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT);
		device->CreateUnorderedAccessView(m_IndexBuffers[i].Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
		//triangle stats
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = 64;
		uavDesc.Buffer.StructureByteStride = 0;
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
		uavDesc.Buffer.FirstElement = 64 + BATCH_COUNT * i;
		uavDesc.Buffer.NumElements = BATCH_COUNT;
		device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	}
	//uavs for clearing
	CD3DX12_CPU_DESCRIPTOR_HANDLE clearHandle(m_CounterClearHeaps[0]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE clearHandle2(m_CounterClearHeaps[1]->GetCPUDescriptorHandleForHeapStart());
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	uavDesc.Buffer.NumElements = 64;
	uavDesc.Buffer.StructureByteStride = 0;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle);
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle2);
	uavDesc.Buffer.FirstElement = 64;
	uavDesc.Buffer.NumElements = BATCH_COUNT;
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle.Offset(1, m_DescHeapIncSize));
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle2.Offset(1, m_DescHeapIncSize));
	uavDesc.Buffer.FirstElement = 64 + BATCH_COUNT;
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle.Offset(1, m_DescHeapIncSize));
	device->CreateUnorderedAccessView(m_CounterBuffer.Get(), nullptr, &uavDesc, clearHandle2.Offset(1, m_DescHeapIncSize));
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = BATCH_COUNT;
	uavDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(m_DrawArgsBuffers[0].Get(), nullptr, &uavDesc, clearHandle.Offset(1, m_DescHeapIncSize));
	device->CreateUnorderedAccessView(m_DrawArgsBuffers[0].Get(), nullptr, &uavDesc, clearHandle2.Offset(1, m_DescHeapIncSize));
	device->CreateUnorderedAccessView(m_DrawArgsBuffers[1].Get(), nullptr, &uavDesc, clearHandle.Offset(1, m_DescHeapIncSize));
	device->CreateUnorderedAccessView(m_DrawArgsBuffers[1].Get(), nullptr, &uavDesc, clearHandle2.Offset(1, m_DescHeapIncSize));

	//index buffer views
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		m_IndexBufferViews[i].BufferLocation = m_IndexBuffers[i]->GetGPUVirtualAddress();
		m_IndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		m_IndexBufferViews[i].SizeInBytes = indexBufferDesc.Width;
	}
}

void FilterContext::Reset(ID3D12Device* device) {
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		m_IndexBuffers[i].Reset();
		m_DrawArgsBuffers[i].Reset();
		m_FilterDescriptorHeaps[i].Reset();
	}
	m_CounterBuffer.Reset();
	Init(device);
}

void FilterContext::Clear(ID3D12GraphicsCommandList* cmdList) {
	m_CurrentBatchCount = 0;
	m_CurrentDraw = 0;
	m_CurrentBatch = 0;
	m_CounterOffset = 0;
	m_DrawCounter = 0;
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	//clear tri stats
	UINT vals[4] = { 0,0,0,0 };
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CounterClearHeaps[0]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_CounterClearHeaps[1]->GetGPUDescriptorHandleForHeapStart());
	cmdList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, m_CounterBuffer.Get(), vals, 0, nullptr);
}

void FilterContext::CopyTriangleStats(ID3D12GraphicsCommandList* cmdList) {
	CD3DX12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE)
	};
	cmdList->ResourceBarrier(_countof(barriers), barriers);
	cmdList->CopyBufferRegion(m_CopyBuffer.Get(), 0, m_CounterBuffer.Get(), 0, 64 * sizeof(UINT));
}

void FilterContext::BeginFilter(ID3D12GraphicsCommandList* cmdList) {
	m_CurrentFilterIndex = ++m_CurrentFilterIndex % MAX_SIMUL_PASSES;
	m_CounterOffset += sizeof(UINT);
	m_DrawCounter = 0;
	CD3DX12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(m_CounterBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
	};
	cmdList->ResourceBarrier(_countof(barriers), barriers);
	g_BufferManager.SwitchState(cmdList, "IndirectBuffer", D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//clear counters
	UINT vals[4] = { 0,0,0,0 };
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CounterClearHeaps[0]->GetCPUDescriptorHandleForHeapStart(), 1 + m_CurrentFilterIndex, m_DescHeapIncSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_CounterClearHeaps[1]->GetGPUDescriptorHandleForHeapStart(), 1 + m_CurrentFilterIndex, m_DescHeapIncSize);
	cmdList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, m_CounterBuffer.Get(), vals, 0, nullptr);
	//clear draw args
	//cmdList->ClearUnorderedAccessViewUint(gpuHandle.Offset(2, m_DescHeapIncSize), cpuHandle.Offset(2, m_DescHeapIncSize), m_DrawArgsBuffers[m_CurrentFilterIndex].Get(), vals, 0, nullptr);
}

void FilterContext::BeginRender(ID3D12GraphicsCommandList* cmdList) {
	m_CurrentRenderIndex = ++m_CurrentRenderIndex % MAX_SIMUL_PASSES;

	CD3DX12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::UAV(m_DrawArgsBuffers[m_CurrentRenderIndex].Get()),
		CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT),
		CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDEX_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetVertexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(g_ModelBank.GetIndexBufferResource(),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER)
	};
	
	cmdList->ResourceBarrier(_countof(barriers), barriers);
	g_BufferManager.SwitchState(cmdList, "IndirectBuffer", D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	cmdList->IASetIndexBuffer(&m_IndexBufferViews[m_CurrentRenderIndex]);
}

// returns wether we are full of geometry or not, batchCountOut returns the number of batches that should be disbatched
bool FilterContext::AddBatches(UINT batchCount, UINT& batchCountOut) {

	//if we have some remainder left of the last batch use that now
	if (m_BatchRemainder > 0) {
		batchCount = m_BatchRemainder;
		m_CurrentBatchCount = 0;
	}
	m_DrawCounter++;
	if (m_CurrentBatchCount + batchCount > BATCH_COUNT) {
		m_BatchRemainder = (m_CurrentBatchCount + batchCount) - BATCH_COUNT;
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

void FilterContext::PrintTriangleStats() {
	if (g_TestParams.Instrument) {
		UINT* stats;
		D3D12_RANGE range = { 0, sizeof(UINT) * 6 };
		m_CopyBuffer->Map(0, &range, (void**)&stats);
		printf("\nTotal Triangle Count: %d\n", stats[0]);
		printf("Backface Count: %d\n", stats[1]);
		printf("Small triangle Count: %d\n", stats[2]);
		printf("Frustum Count: %d\n", stats[3]);
		printf("Occlusion Count: %d\n", stats[4]);
		printf("Total surviving triangles %d\n\n", stats[5]);
		range.End = 0;
		m_CopyBuffer->Unmap(0, &range);
	}
}

ID3D12Resource* FilterContext::GetDrawArgsResource(int index) {
	return m_DrawArgsBuffers[index].Get();
}