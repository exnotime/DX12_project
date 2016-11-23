#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
//The filter context synchronises the rendering and the culling as well as hold the resources
#define MAX_SIMUL_PASSES 2
class FilterContext {
public:
	FilterContext();
	~FilterContext();
	void Init(ID3D12Device* device);
	void Reset(ID3D12Device* device);
	void Clear(ID3D12GraphicsCommandList* cmdList);
	void BeginFilter(ID3D12GraphicsCommandList* cmdList);
	void BeginRender(ID3D12GraphicsCommandList* cmdList);
	bool AddBatches(UINT batchCount, UINT& batchCountOut);
	void CopyTriangleStats(ID3D12GraphicsCommandList* cmdList);
	void PrintTriangleStats();
	void Debug();
	ID3D12Resource* GetDrawArgsResource(int index);

	UINT GetFilterIndex() {return m_CurrentFilterIndex;}
	UINT GetRenderIndex() {return m_CurrentRenderIndex;}
	D3D12_CPU_DESCRIPTOR_HANDLE GetFilterDescriptors() {return m_FilterDescriptorHeaps[m_CurrentFilterIndex]->GetCPUDescriptorHandleForHeapStart();}
	ID3D12Resource* GetCounterResource() {return m_CounterBuffer.Get();}
	UINT GetCurrentBatch() {return m_CurrentBatch;}
	UINT GetCurrentDraw() {return m_CurrentDraw;}
	UINT GetBatchCount() {return m_CurrentBatchCount;}
	UINT GetRemainder() {return m_BatchRemainder;}
	UINT GetCounterOffset() {return m_CounterOffset;}
	void IncrementDrawCounter() {m_CurrentDraw++;}
	UINT GetDrawCounter() { return m_DrawCounter; }

private:
	ComPtr<ID3D12Resource> m_IndexBuffers[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource> m_DrawArgsBuffers[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource> m_CounterBuffer;
	ComPtr<ID3D12Resource> m_CopyBuffer;

	ComPtr<ID3D12DescriptorHeap> m_FilterDescriptorHeaps[MAX_SIMUL_PASSES];

	ComPtr<ID3D12DescriptorHeap> m_CounterClearHeaps[2];

	D3D12_INDEX_BUFFER_VIEW m_IndexBufferViews[MAX_SIMUL_PASSES];

	UINT m_CurrentFilterIndex = 0;
	UINT m_CurrentRenderIndex = 0;

	UINT m_CurrentDraw = 0;
	UINT m_CurrentBatch = 0;
	UINT m_CurrentBatchCount = 0;
	UINT m_BatchRemainder = 0;
	UINT m_DescHeapIncSize = 0;
	UINT m_CounterOffset = 0;
	UINT m_DrawCounter = 0;

	FILE* m_Files[6]; //files for writing out triangle stats to files
};