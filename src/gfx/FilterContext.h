#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
//The filter context synchronises the rendering and the culling as well as hold the resources
#define MAX_SIMUL_PASSES 2
#define BATCH_SIZE 512
#define MAX_BATCH_COUNT 512
#define MAX_INDEX_BUFFER_SIZE BATCH_SIZE * 3 * MAX_BATCH_COUNT * sizeof(UINT)
#define MAX_DRAW_ARGS_BUFFER_SIZE MAX_BATCH_COUNT * sizeof(IndirectDrawCall)
class FilterContext {
public:
	FilterContext();
	~FilterContext();
	void Init(DX12Context* context);
	void Clear();
	void BeginFilter(ID3D12GraphicsCommandList* cmdList);
	void BeginRender(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut);
	bool AddBatches(UINT batchCount, UINT& batchCountOut);

	UINT GetFilterIndex() {
		return m_CurrentFilterIndex;
	}
	UINT GetCounterRenderOffset() {
		return m_CurrentRenderIndex * sizeof(UINT);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetFilterDescriptors() {
		return m_FilterDescriptorHeaps[m_CurrentFilterIndex]->GetCPUDescriptorHandleForHeapStart();
	}
	ID3D12Resource* GetCounterResource() {
		return m_CounterBuffer.Get();
	}
	UINT GetCurrentBatch() {
		return m_CurrentBatch;
	}
	UINT GetCurrentDraw() {
		return m_CurrentDraw;
	}
	UINT GetBatchCount() {
		return m_CurrentBatchCount;
	}
	UINT GetRemainder() {
		return m_BatchRemainder;
	}
	void IncrementDrawCounter() {
		m_CurrentDraw++;
	}
private:
	ComPtr<ID3D12Resource> m_IndexBuffers[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource> m_DrawArgsBuffers[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource> m_CounterBuffer;

	ComPtr<ID3D12DescriptorHeap> m_FilterDescriptorHeaps[MAX_SIMUL_PASSES];

	D3D12_INDEX_BUFFER_VIEW m_IndexBufferViews[MAX_SIMUL_PASSES];

	UINT m_CurrentFilterIndex = 0;
	UINT m_CurrentRenderIndex = 0;

	UINT m_CurrentDraw = 0;
	UINT m_CurrentBatch = 0;
	UINT m_CurrentBatchCount = 0;
	UINT m_BatchRemainder = 0;

	UINT m_DescHeapIncSize = 0;
};