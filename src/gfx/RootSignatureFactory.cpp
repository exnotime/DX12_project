#include "RootSignatureFactory.h"
RootSignatureFactory::RootSignatureFactory() {

}
RootSignatureFactory::~RootSignatureFactory() {

}

void RootSignatureFactory::AddConstantBufferView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	CD3DX12_ROOT_PARAMETER rootParameter;
	rootParameter.InitAsConstantBufferView(shaderRegister, registerSpace, shaderVisibility);
	m_RootParamters.push_back(rootParameter);
}

void RootSignatureFactory::AddConstant(UINT count, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	CD3DX12_ROOT_PARAMETER rootParameter;
	rootParameter.InitAsConstants(count, shaderRegister, registerSpace, shaderVisibility);
	m_RootParamters.push_back(rootParameter);
}

void RootSignatureFactory::AddDescriptorTable(const std::vector<CD3DX12_DESCRIPTOR_RANGE>& ranges, D3D12_SHADER_VISIBILITY shaderVisibility) {
	CD3DX12_ROOT_PARAMETER rootParameter;
	rootParameter.InitAsDescriptorTable(ranges.size(), ranges.data(), shaderVisibility);
	m_RootParamters.push_back(rootParameter);
}

void RootSignatureFactory::AddShaderResourceView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	CD3DX12_ROOT_PARAMETER rootParameter;
	rootParameter.InitAsShaderResourceView(shaderRegister, registerSpace, shaderVisibility);
	m_RootParamters.push_back(rootParameter);
}

void RootSignatureFactory::AddUnorderedAccesView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	CD3DX12_ROOT_PARAMETER rootParameter;
	rootParameter.InitAsUnorderedAccessView(shaderRegister, registerSpace, shaderVisibility);
	m_RootParamters.push_back(rootParameter);
}

void RootSignatureFactory::AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& samplerDesc) {
	m_SamplerDesc = samplerDesc;
}

void RootSignatureFactory::AddDefaultStaticSampler(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	m_SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	m_SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_SamplerDesc.MipLODBias = 0;
	m_SamplerDesc.MaxAnisotropy = 0;
	m_SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	m_SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	m_SamplerDesc.MinLOD = 0.0f;
	m_SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_SamplerDesc.ShaderRegister = shaderRegister;
	m_SamplerDesc.RegisterSpace = registerSpace;
	m_SamplerDesc.ShaderVisibility = shaderVisibility;
}

ComPtr<ID3D12RootSignature> RootSignatureFactory::CreateSignture(ID3D12Device* device) {
	CD3DX12_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.Init(m_RootParamters.size(), m_RootParamters.data(), 1, &m_SamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ComPtr<ID3D12RootSignature> rootSignature;
	if (D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error) != S_OK) {
		printf("Error serializing rootsignature\nErrorLog:%s\n", error->GetBufferPointer());
	}
	HR(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), L"Error creating rootsignature\n");
	return rootSignature;
}