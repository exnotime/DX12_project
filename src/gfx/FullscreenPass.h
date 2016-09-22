#pragma once
#include "Common.h"
#include "Shader.h"
class FullscreenPass {
public:
	FullscreenPass();
	~FullscreenPass();
	void Init(DX12Context* context);
	void Render(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_DESCRIPTOR_HANDLE texHandle);
private:
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
};