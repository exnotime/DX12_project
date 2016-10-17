#include "RootSignatureFactory.h"
RootSignatureFactory::RootSignatureFactory() {
	m_ExtensionData = nullptr;
	m_Samplers.reserve(1);
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
	m_Samplers.push_back(samplerDesc);
}

void RootSignatureFactory::AddDefaultStaticSampler(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility) {
	D3D12_STATIC_SAMPLER_DESC sampDesc;
	sampDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.MipLODBias = 0;
	sampDesc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
	sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampDesc.MinLOD = 0.0f;
	sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	sampDesc.ShaderRegister = shaderRegister;
	sampDesc.RegisterSpace = registerSpace;
	sampDesc.ShaderVisibility = shaderVisibility;

	m_Samplers.push_back(sampDesc);
}

void RootSignatureFactory::AddExtensions(ExtensionContext* extensions) {
	CD3DX12_ROOT_PARAMETER rootParam;
	//AMD
	if (extensions->Vendor == AMD_VENDOR_ID) {
		
		//range must live until the root signature is created
		m_ExtensionData = malloc(sizeof(CD3DX12_DESCRIPTOR_RANGE));
		((CD3DX12_DESCRIPTOR_RANGE*)m_ExtensionData)->Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, AGS_DX12_SHADER_INSTRINSICS_SPACE_ID);
		rootParam.InitAsDescriptorTable(1, (CD3DX12_DESCRIPTOR_RANGE*)m_ExtensionData, D3D12_SHADER_VISIBILITY_ALL);
		m_RootParamters.push_back(rootParam);
	}
	else if (extensions->Vendor == NVIDIA_VENDOR_ID) {
		//range must live until the root signature is created
		m_ExtensionData = malloc(sizeof(CD3DX12_DESCRIPTOR_RANGE));
		((CD3DX12_DESCRIPTOR_RANGE*)m_ExtensionData)->Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, NVIDIA_EXTENSION_SLOT, NVIDIA_EXTENSION_SPACE);
		rootParam.InitAsDescriptorTable(1, (CD3DX12_DESCRIPTOR_RANGE*)m_ExtensionData, D3D12_SHADER_VISIBILITY_ALL);
		m_RootParamters.push_back(rootParam);
	}
}

ComPtr<ID3D12RootSignature> RootSignatureFactory::CreateSignture(ID3D12Device* device) {
	CD3DX12_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.Init(m_RootParamters.size(), m_RootParamters.data(), m_Samplers.size(), m_Samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ComPtr<ID3D12RootSignature> rootSignature;
	if (D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error) != S_OK) {
		printf("Error serializing rootsignature\nErrorLog:%s\n", error->GetBufferPointer());
	}
	HR(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), L"Error creating rootsignature\n");

	if (m_ExtensionData)
		free(m_ExtensionData);
	return rootSignature;
}