#pragma once
#include "DX12Common.h"

class TriangleCullingProgram {
public:
	TriangleCullingProgram();
	~TriangleCullingProgram();
	void Init(DX12Context* context);
	void Disbatch();
private:
	DX12Context* m_Context;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
};