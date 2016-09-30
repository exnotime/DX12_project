#include "DepthOnlyProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "Vertex.h"
#include "BufferManager.h"
#include "ModelBank.h"

DepthOnlyProgram::DepthOnlyProgram() {

}

DepthOnlyProgram::~DepthOnlyProgram() {

}

void DepthOnlyProgram::Init(DX12Context* context, glm::vec2 screenSize) {
	m_ScreenSize = screenSize * 0.25f;

	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.Width = m_ScreenSize.x;
	m_Viewport.Height = m_ScreenSize.y;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.bottom = (unsigned)m_ScreenSize.y;
	m_ScissorRect.right = (unsigned)m_ScreenSize.x;


	m_Shader.LoadFromFile(L"src/shaders/DepthOnly.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT, nullptr);

	//Shader testShader;
	//testShader.LoadFromFile(L"src/shaders/ComputeTest.hlsl", COMPUTE_SHADER_BIT, &context->Extensions);

	RootSignatureFactory rootSignFact;
	rootSignFact.AddDefaultStaticSampler(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
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
			rootSignFact.AddConstant(2, 2, 0, D3D12_SHADER_VISIBILITY_ALL);
			break;
		case DepthOnlyProgram::MATERIAL_DT:
			CD3DX12_DESCRIPTOR_RANGE range = {};
			range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4000, 0, 1);
			ranges.push_back(range);
			rootSignFact.AddDescriptorTable(ranges);
			break;
		}
	}

	m_RootSignature = rootSignFact.CreateSignture(context->Device.Get());
	
	PipelineStateFactory pipeFact;

	pipeFact.SetShader(m_Shader.GetByteCode(VERTEX_SHADER_BIT), VERTEX_SHADER_BIT);
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

	pipeFact.SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);
	pipeFact.SetDepthStencilState(depthDesc);
	pipeFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipeFact.SetInputLayout(VertexLayout, VertexLayoutSize);
	pipeFact.SetRootSignature(m_RootSignature.Get());

	m_PipelineState = pipeFact.CreateGraphicsState(context);
	//Material desc heap
	D3D12_DESCRIPTOR_HEAP_DESC renderHeapDesc = {};
	renderHeapDesc.NumDescriptors = 4000;
	renderHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	renderHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(context->Device->CreateDescriptorHeap(&renderHeapDesc, IID_PPV_ARGS(&m_MaterialHeap)), L"Error creating descriptor heap for Rendering");

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
	HR(context->Device->CreateCommandSignature(&signDesc, m_RootSignature.Get(), IID_PPV_ARGS(&m_CommandSignature)), L"Error creating Command signature");

	D3D12_RESOURCE_DESC dsvResDesc = {};
	dsvResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsvResDesc.Width = (unsigned)m_ScreenSize.x;
	dsvResDesc.Height = (unsigned)m_ScreenSize.y;
	dsvResDesc.DepthOrArraySize = 1;
	dsvResDesc.MipLevels = 1;
	dsvResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsvResDesc.SampleDesc.Count = 1;
	dsvResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsvResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.DepthStencil.Depth = 1.0f;
	clearVal.DepthStencil.Stencil = 0x0;
	clearVal.Format = DXGI_FORMAT_D32_FLOAT;

	HR(context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&dsvResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal, IID_PPV_ARGS(&m_DepthTexture)), L"Error creating depth resource");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(context->Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DepthHeap)), L"Error creating descriptor heap for DSV");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DepthHeap->GetCPUDescriptorHandleForHeapStart());
	context->Device->CreateDepthStencilView(m_DepthTexture.Get(), &dsvDesc, dsvHandle);
}

void DepthOnlyProgram::Render(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue) {
	cmdList->OMSetRenderTargets(0, nullptr, false, &m_DepthHeap->GetCPUDescriptorHandleForHeapStart());
	cmdList->ClearDepthStencilView(m_DepthHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0x0, 0, nullptr);
	cmdList->RSSetViewports(1, &m_Viewport);
	cmdList->RSSetScissorRects(1, &m_ScissorRect);

	cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(DepthOnlyProgram::PER_FRAME_CB, g_BufferManager.GetGPUHandle("cbPerFrame"));
	cmdList->SetGraphicsRootShaderResourceView(DepthOnlyProgram::SHADER_INPUT_SB, g_BufferManager.GetGPUHandle("ShaderInputBuffer"));
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_ModelBank.ApplyBuffers(cmdList);
	ID3D12DescriptorHeap* heaps[] = { m_MaterialHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->SetGraphicsRootDescriptorTable(DepthOnlyProgram::MATERIAL_DT, m_MaterialHeap->GetGPUDescriptorHandleForHeapStart());

	//draw everything
	cmdList->ExecuteIndirect(m_CommandSignature.Get(), queue->GetOccluderCount(), g_BufferManager.GetBufferResource("IndirectOccluderBuffer"), 0, nullptr, 0);
}