#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
#include "Shader.h"
#include "HiZProgram.h"
#include "FilterContext.h"

class TriangleCullingProgram {
public:
	TriangleCullingProgram();
	~TriangleCullingProgram();
	void Init(DX12Context* context);
	void Reset(DX12Context* context);
	void CreateDescriptorTable(HiZProgram* hizProgram);
	bool Disbatch(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue, FilterContext* filterContext);
	std::vector<D3D_SHADER_MACRO> BuildMacros();
private:
	DX12Context* m_Context;
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSign;
	ComPtr<ID3D12PipelineState> m_PipeState;
	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	UINT m_DescHeapIncSize;

	UINT m_BatchSize;
	UINT m_BatchCount;

	enum ROOT_PARAMS {
		PER_FRAME_CB,
		CONSTANTS_C,
		INPUT_DT,
		OUTPUT_DT,
		EXTENSIONS_DESC,
		ROOT_PARAM_COUNT
	};
};