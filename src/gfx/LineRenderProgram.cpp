#include "LineRenderProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
LineRenderProgram::LineRenderProgram() {

}

LineRenderProgram::~LineRenderProgram() {

}

void LineRenderProgram::Init(ID3D12Device* device) {
	m_Shader.LoadFromFile(L"src/shaders/Line.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT);

	RootSignatureFactory rootFact;
	rootFact.AddConstantBufferView(0); //camera matrix
	rootFact.AddConstantBufferView(1); //line color
	m_RootSignature = rootFact.CreateSignture(device);
}