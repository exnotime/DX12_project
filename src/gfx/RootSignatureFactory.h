#pragma once
#include "Common.h"
class RootSignatureFactory {
public:
	RootSignatureFactory();
	~RootSignatureFactory();

	void AddConstantBufferView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddConstant(UINT count, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddDescriptorTable(const std::vector<CD3DX12_DESCRIPTOR_RANGE>& ranges, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddShaderResourceView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddUnorderedAccesView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& samplerDesc);
	void AddDefaultStaticSampler(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
	void AddExtensions(ExtensionContext* extensions);

	ComPtr<ID3D12RootSignature> CreateSignture(ID3D12Device* device);
private:
	std::vector<CD3DX12_ROOT_PARAMETER> m_RootParamters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_Samplers;
	//Extensions data
	void* m_ExtensionData;
};