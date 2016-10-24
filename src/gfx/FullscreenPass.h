#pragma once
#include "DX12Common.h"
#include "Shader.h"
class FullscreenPass {
public:
	FullscreenPass();
	~FullscreenPass();
	void Init(DX12Context* context);
	void Render(ID3D12GraphicsCommandList* cmdList);
	//temp maybe
	void CreateSRV(DX12Context* context, ID3D12Resource* resource, DXGI_FORMAT format, UINT mipCount = 1);

private:
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
};