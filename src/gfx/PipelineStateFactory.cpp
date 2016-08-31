#include "PipelineStateFactory.h"

PipelineStateFactory::PipelineStateFactory() {
	//set default values for some of the variables
	m_PipelineStateDesc = {};
	m_PipelineStateDesc.SampleDesc.Count = 1;
	m_PipelineStateDesc.SampleMask = UINT_MAX;
	m_PipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_PipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_PipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}
PipelineStateFactory::~PipelineStateFactory() {

}

void PipelineStateFactory::SetInputLayout(const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT layoutCount) {
	m_PipelineStateDesc.InputLayout = { inputLayout, layoutCount };
}
void PipelineStateFactory::SetRootSignature(ID3D12RootSignature* rootSignature) {
	m_PipelineStateDesc.pRootSignature = rootSignature;
}
void PipelineStateFactory::SetShader(const D3D12_SHADER_BYTECODE& byteCode, SHADER_TYPES shaderType) {
	switch (shaderType) {
		case VERTEX_SHADER_BIT:
			m_PipelineStateDesc.VS = byteCode;
			break;
		case PIXEL_SHADER_BIT:
			m_PipelineStateDesc.PS = byteCode;
			break;
		case GEOMETRY_SHADER_BIT:
			m_PipelineStateDesc.GS = byteCode;
			break;
		case HULL_SHADER_BIT:
			m_PipelineStateDesc.HS = byteCode;
			break;
		case DOMAIN_SHADER_BIT:
			m_PipelineStateDesc.DS = byteCode;
			break;
		default:
			break;
	}
}

void PipelineStateFactory::SetRasterizerState(const CD3DX12_RASTERIZER_DESC& rasterState) {
	m_PipelineStateDesc.RasterizerState = rasterState;
}
void PipelineStateFactory::SetBlendState(const CD3DX12_BLEND_DESC& blendState) {
	m_PipelineStateDesc.BlendState = blendState;
}
void PipelineStateFactory::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilState){
	m_PipelineStateDesc.DepthStencilState = depthStencilState;
}
void PipelineStateFactory::SetDepthStencilFormat(const DXGI_FORMAT& format) {
	m_PipelineStateDesc.DSVFormat = format;
}

void PipelineStateFactory::SetSampleMask(UINT val) {
	m_PipelineStateDesc.SampleMask = val;
}
void PipelineStateFactory::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
	m_PipelineStateDesc.PrimitiveTopologyType = topology;
}
void PipelineStateFactory::SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& formats) {
	m_PipelineStateDesc.NumRenderTargets = formats.size();
	for (int i = 0; i < formats.size(); ++i) {
		m_PipelineStateDesc.RTVFormats[i] = formats[i];
	}
}
void PipelineStateFactory::SetSampleCount(UINT count) {
	m_PipelineStateDesc.SampleDesc.Count = count;
}

ComPtr<ID3D12PipelineState> PipelineStateFactory::Create(ID3D12Device* device) {
	ComPtr<ID3D12PipelineState> pipelineState;
	HR(device->CreateGraphicsPipelineState(&m_PipelineStateDesc, IID_PPV_ARGS(&pipelineState)),L"Error creating pipeline state");
	return pipelineState;
}