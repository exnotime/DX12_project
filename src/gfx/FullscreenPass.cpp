#include "FullscreenPass.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"

FullscreenPass::FullscreenPass() {

}

FullscreenPass::~FullscreenPass() {

}

void FullscreenPass::Init(DX12Context* context) {
	m_Shader.LoadFromFile(L"src/shaders/Fullscreen.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT);

	RootSignatureFactory rootFact;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootFact.AddStaticSampler(samplerDesc);

	std::vector<CD3DX12_DESCRIPTOR_RANGE> descRanges;
	CD3DX12_DESCRIPTOR_RANGE range;
	range.BaseShaderRegister = 0;
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	descRanges.push_back(range);
	rootFact.AddDescriptorTable(descRanges);

	m_RootSignature = rootFact.CreateSignture(context->Device.Get());

	PipelineStateFactory pipeFact;
	pipeFact.SetRootSignature(m_RootSignature.Get());
	pipeFact.SetAllShaders(m_Shader);
	std::vector<DXGI_FORMAT> formats;
	formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipeFact.SetRenderTargetFormats(formats);
	pipeFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_PipelineState = pipeFact.Create(context->Device.Get());

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.NodeMask = 0;

	context->Device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
}

void FullscreenPass::Render(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_DESCRIPTOR_HANDLE texHandle) {
	cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	cmdList->SetPipelineState(m_PipelineState.Get());
	ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	cmdList->SetComputeRootDescriptorTable(1, texHandle);

	cmdList->DrawInstanced(4, 1, 0, 0);
}