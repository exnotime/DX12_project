#include "PipelineStateFactory.h"
#include <NVAPI/nvapi.h>
PipelineStateFactory::PipelineStateFactory() {
	//set default values for some of the variables
	m_PipelineStateDesc = {};
	m_PipelineStateDesc.NumRenderTargets = 0;
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

void PipelineStateFactory::SetShader(const D3D12_SHADER_BYTECODE& byteCode, UINT shaderType) {
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
		case COMPUTE_SHADER_BIT:
			break;
			m_ComputeStateDesc.CS = byteCode;
		default:
			break;
	}
}

void PipelineStateFactory::SetAllShaders(const Shader& shader) {
	for (UINT i = 0; i < 6; ++i) {
		UINT flag = 1 << i;
		if ((shader.GetShaderTypes() & flag) == flag)
			SetShader(shader.GetByteCode(flag), flag);
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
	m_RenderTargetFormats.insert(m_RenderTargetFormats.end(), formats.begin(), formats.end());
}

void PipelineStateFactory::AddRenderTargetFormat(DXGI_FORMAT format) {
	m_RenderTargetFormats.push_back(format);
}

void PipelineStateFactory::SetSampleCount(UINT count) {
	m_PipelineStateDesc.SampleDesc.Count = count;
}

ComPtr<ID3D12PipelineState> PipelineStateFactory::CreateGraphicsState(DX12Context* context) {
	ComPtr<ID3D12PipelineState> pipelineState;

	for (int i = 0; i < m_RenderTargetFormats.size(); i++) {
		m_PipelineStateDesc.RTVFormats[i] = m_RenderTargetFormats[i];
	}
	m_PipelineStateDesc.NumRenderTargets = m_RenderTargetFormats.size();

	if (context->Extensions.Vendor == NVIDIA_VENDOR_ID) {
		NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC extensionDesc;
		extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
		extensionDesc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
		extensionDesc.registerSpace = NVIDIA_EXTENSION_SPACE;
		extensionDesc.uavSlot = NVIDIA_EXTENSION_SLOT;

		const NVAPI_D3D12_PSO_EXTENSION_DESC* pExtensions[] = { &extensionDesc };

		NvAPI_D3D12_CreateGraphicsPipelineState(context->Device.Get(), &m_PipelineStateDesc, 1, pExtensions, &pipelineState);

	} else {
		HR(context->Device->CreateGraphicsPipelineState(&m_PipelineStateDesc, IID_PPV_ARGS(&pipelineState)), L"Error creating pipeline state");
	}
	return pipelineState;
}

ComPtr<ID3D12PipelineState> PipelineStateFactory::CreateComputeState(DX12Context* context) {
	ComPtr<ID3D12PipelineState> pipelineState;

	m_ComputeStateDesc.pRootSignature = m_PipelineStateDesc.pRootSignature;

	if (context->Extensions.Vendor == NVIDIA_VENDOR_ID) {
		NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC extensionDesc;
		extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
		extensionDesc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
		extensionDesc.registerSpace = NVIDIA_EXTENSION_SPACE;
		extensionDesc.uavSlot = NVIDIA_EXTENSION_SLOT;

		const NVAPI_D3D12_PSO_EXTENSION_DESC* pExtensions[] = { &extensionDesc };

		NvAPI_D3D12_CreateComputePipelineState(context->Device.Get(), &m_ComputeStateDesc, 1, pExtensions, &pipelineState);

	}
	else {
		HR(context->Device->CreateComputePipelineState(&m_ComputeStateDesc, IID_PPV_ARGS(&pipelineState)), L"Error creating pipeline state");
	}
	return pipelineState;
}