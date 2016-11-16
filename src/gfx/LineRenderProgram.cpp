#include "LineRenderProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
LineRenderProgram::LineRenderProgram() {

}

LineRenderProgram::~LineRenderProgram() {

}

void LineRenderProgram::Init(DX12Context* context) {
	m_Shader.LoadFromFile(L"src/shaders/Line.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT);

	RootSignatureFactory rootFact;
	rootFact.AddConstantBufferView(0); //camera matrix
	rootFact.AddConstantBufferView(1); //line color
	m_RootSignature = rootFact.CreateSignture(context->Device.Get());

	const D3D12_INPUT_ELEMENT_DESC lineLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	PipelineStateFactory pipeFact;
	pipeFact.SetAllShaders(m_Shader);
	pipeFact.SetInputLayout(lineLayout, 1);
	pipeFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
	pipeFact.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipeFact.SetDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	pipeFact.SetRootSignature(m_RootSignature.Get());
	m_PipelineState = pipeFact.CreateGraphicsState(context);
}

void LineRenderProgram::Render(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue) {

	cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	cmdList->set
}