#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
#include "Shader.h"
#include "HiZProgram.h" 
#define MAX_TRIANGLE_COUNT 1000 * 1000

class TriangleCullingProgram {
public:
	TriangleCullingProgram();
	~TriangleCullingProgram();
	void Init(DX12Context* context, const UINT maxTriangleCount = MAX_TRIANGLE_COUNT, const UINT batchSize = 256);
	void CreateDescriptorTable(HiZProgram* hizProgram);
	void Disbatch(RenderQueue* queue);

	ID3D12Resource* GetDrawArgsBuffer() {
		return m_CulledDrawArgsBuffer.Get();
	}
	D3D12_INDEX_BUFFER_VIEW& GetCulledIndexBufferView() {
		return m_IBOView;
	}
	UINT GetDrawCount() {
		return m_BatchCount;
	}
	UINT GetMaxBatchCount() {
		return m_MaxBatchCount;
	}
private:
	DX12Context* m_Context;
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSign;
	ComPtr<ID3D12PipelineState> m_PipeState;
	ComPtr<ID3D12Resource> m_CulledIndexBuffer;
	ComPtr<ID3D12Resource> m_CulledDrawArgsBuffer;
	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	UINT m_DescHeapIncSize;
	D3D12_INDEX_BUFFER_VIEW m_IBOView;

	UINT m_BatchSize;
	UINT m_MaxTriangleCount;
	UINT m_MaxBatchCount;
	UINT m_BatchCount;

	IndirectDrawCall* m_DrawListArray;
	enum ROOT_PARAMS {
		PER_FRAME_CB,
		CONSTANTS_C,
		INPUT_DT,
		ROOT_PARAM_COUNT
	};
};