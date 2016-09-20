#include "DepthOnlyProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "Vertex.h"
#include "BufferManager.h"
#include "ModelBank.h"
void InitDepthOnlyState(DepthOnlyProgram::DepthOnlyState* state, DX12Context* context) {

	state->Shader.LoadFromFile(L"src/shaders/DepthOnly.hlsl", VERTEX_SHADER_BIT);
	RootSignatureFactory rootSignFact;
	for (int i = 0; i < DepthOnlyProgram::ROOT_PARAMS_SIZE; ++i) {
		switch (i)
		{
		case DepthOnlyProgram::PER_FRAME_CB:
			rootSignFact.AddConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			break;
		case DepthOnlyProgram::SHADER_INPUT_SB:
			rootSignFact.AddShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			break;
		case DepthOnlyProgram::DRAW_INDEX_C:
			rootSignFact.AddConstant(2, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			break;
		}
	}

	state->RootSignature = rootSignFact.CreateSignture(context->Device.Get());
	
	PipelineStateFactory pipeFact;

	pipeFact.SetShader(state->Shader.GetByteCode(VERTEX_SHADER_BIT), VERTEX_SHADER_BIT);
	D3D12_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthDesc.StencilEnable = true;
	depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	pipeFact.SetDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	pipeFact.SetDepthStencilState(depthDesc);
	pipeFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipeFact.SetInputLayout(DepthOnlyVertexLayout, DepthOnlyVertexSize);
	pipeFact.SetRootSignature(state->RootSignature.Get());

	state->PipelineState = pipeFact.Create(context->Device.Get());

	D3D12_INDIRECT_ARGUMENT_DESC argsDesc[3];
	//draw ID
	argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argsDesc[0].Constant.RootParameterIndex = DepthOnlyProgram::DRAW_INDEX_C;
	argsDesc[0].Constant.Num32BitValuesToSet = 1;
	argsDesc[0].Constant.DestOffsetIn32BitValues = 0;
	//material ID
	argsDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argsDesc[1].Constant.RootParameterIndex = DepthOnlyProgram::DRAW_INDEX_C;
	argsDesc[1].Constant.Num32BitValuesToSet = 1;
	argsDesc[1].Constant.DestOffsetIn32BitValues = 1;
	//draw args
	argsDesc[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC signDesc;
	signDesc.ByteStride = sizeof(IndirectDrawCall);
	signDesc.NumArgumentDescs = 3;
	signDesc.pArgumentDescs = argsDesc;
	signDesc.NodeMask = 0;
	HR(context->Device->CreateCommandSignature(&signDesc, state->RootSignature.Get(), IID_PPV_ARGS(&state->CommandSignature)), L"Error creating Command signature");
}

void DepthOnlyRender(ID3D12GraphicsCommandList*cmdList, DepthOnlyProgram::DepthOnlyState* state, RenderQueue* queue) {
	cmdList->SetGraphicsRootSignature(state->RootSignature.Get());

	cmdList->SetGraphicsRootConstantBufferView(DepthOnlyProgram::PER_FRAME_CB, g_BufferManager.GetGPUHandle("cbPerFrame"));
	cmdList->SetGraphicsRootShaderResourceView(DepthOnlyProgram::SHADER_INPUT_SB, g_BufferManager.GetGPUHandle("ShaderInputBuffer"));
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_ModelBank.ApplyBuffers(cmdList);
	//draw everything
	cmdList->ExecuteIndirect(state->CommandSignature.Get(), queue->GetDrawCount(), queue->GetArgumentBuffer(), 0, nullptr, 0);
}