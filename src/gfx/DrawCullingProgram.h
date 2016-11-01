#pragma once
#include "DX12Common.h"
#include "Shader.h"
#include "RenderQueue.h"
#include "FilterContext.h"
class DrawCullingProgram {
public:
	DrawCullingProgram();
	~DrawCullingProgram();
	void Init(DX12Context* context, FilterContext* filterContext);
	void Reset(DX12Context* context, FilterContext* filterContext);
	void Disbatch(ID3D12GraphicsCommandList* cmdList, FilterContext* filterContext);
	void ClearCounters(ID3D12GraphicsCommandList* cmdList);
	ID3D12Resource* GetDrawArgsBuffer(int index) {
		return m_OutputBuffer[index].Get();
	}

	ID3D12Resource* GetCounterBuffer() {
		return m_CounterBuffer.Get();
	}
private:

	enum ROOT_PARAMS {
		INPUT_DESC,
		INPUT_COUNT_C,
		ROOT_PARAM_COUNT
	};
	Shader							m_Shader;
	ComPtr<ID3D12RootSignature>		m_RootSignature;
	ComPtr<ID3D12PipelineState>		m_PipelineState;
	ComPtr<ID3D12Resource>			m_OutputBuffer[MAX_SIMUL_PASSES];
	ComPtr<ID3D12Resource>			m_CounterBuffer;
	ComPtr<ID3D12DescriptorHeap>	m_DescHeap;
	ComPtr<ID3D12DescriptorHeap>	m_CounterClearHeaps[2];
	UINT							m_HeapDescIncSize;
	UINT							m_WaveSize;
	const UINT DESC_HEAP_SIZE = 3;
};