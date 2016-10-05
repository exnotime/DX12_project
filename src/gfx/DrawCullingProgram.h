#pragma once
#include "DX12Common.h"
#include "Shader.h"
#include "RenderQueue.h"
class DrawCullingProgram {
public:
	DrawCullingProgram();
	~DrawCullingProgram();
	void Init(DX12Context* context);
	void Disbatch(RenderQueue* queue);
private:

	enum ROOT_PARAMS {
		INPUT_DESC,
		INPUT_COUNT_C,
		ROOT_PARAM_COUNT
	};

	DX12Context*					m_Context;
	Shader							m_Shader;
	ComPtr<ID3D12RootSignature>		m_RootSignature;
	ComPtr<ID3D12PipelineState>		m_PipelineState;
	ComPtr<ID3D12DescriptorHeap>	m_GPUDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap>	m_CPUDescriptorHeap;
	ComPtr<ID3D12Resource>			m_OutputBuffer;
	ComPtr<ID3D12Resource>			m_CounterBuffer;
	UINT							m_DescIncSize;
};