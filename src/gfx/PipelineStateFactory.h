#pragma once
#include "Common.h"
#include "Shader.h"
class PipelineStateFactory {
public:
	PipelineStateFactory();
	~PipelineStateFactory();

	void SetInputLayout(const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT layoutCount);
	void SetRootSignature(ID3D12RootSignature* rootSignature);
	void SetShader(const D3D12_SHADER_BYTECODE& byteCode, SHADER_TYPES shaderType);
	void SetRasterizerState(const CD3DX12_RASTERIZER_DESC& rasterState);
	void SetBlendState(const CD3DX12_BLEND_DESC& blendState);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilState);
	void SetDepthStencilFormat(const DXGI_FORMAT& format);
	void SetSampleMask(UINT val);
	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
	void SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& formats);
	void SetSampleCount(UINT count);

	ComPtr<ID3D12PipelineState> Create(ID3D12Device* device);
private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PipelineStateDesc;
};