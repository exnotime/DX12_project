#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
//The filter context synchronises the rendering and the culling as well as hold the resources
#define MAX_SIMUL_PASSES 2
#define BATCH_SIZE 256
#define MAX_BATCH_COUNT 1024
#define MAX_INDEX_BUFFER_SIZE BATCH_SIZE * 3 * MAX_BATCH_COUNT * sizeof(UINT)
#define MAX_DRAW_ARGS_BUFFER_SIZE MAX_BATCH_COUNT * sizeof(IndirectDrawCall)
class FilterContext {
public:
	FilterContext();
	~FilterContext();
	void Init(DX12Context* context);
	void Clear();
	void BeginFilter(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut, ID3D12Resource** indexBufferOut);
	void BeginRender(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut, ID3D12Resource** indexBufferOut);
private:
	ComPtr<ID3D12Resource> m_IndexBuffers[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource> m_DrawArgsBuffers[MAX_SIMUL_PASSES];

	UINT m_CurrentFilterIndex = 0;
	UINT m_CurrentRenderIndex = 0;
	UINT m_CurrentBatch = 0;
};