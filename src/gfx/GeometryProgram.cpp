#include "GeometryProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "Vertex.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "BufferManager.h"
#include "TestParams.h"

using namespace GeometryProgram;

void InitGeometryState(GeometryProgramState* state, DX12Context* context, ID3D12GraphicsCommandList* cmdList) {
	state->DescHeapIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	state->Shader.LoadFromFile(L"src/shaders/Color.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT, nullptr);
	RootSignatureFactory rootSignFactory;
	rootSignFactory.AddDefaultStaticSampler(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignFactory.AddConstantBufferView(0); //0
	rootSignFactory.AddShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX); //1
	rootSignFactory.AddConstant(2, 2); //2

	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE range = {};
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1);
	ranges.push_back(range);
	rootSignFactory.AddDescriptorTable(ranges); //3 //environment material

	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges2;
	range = {};
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4000, 0, 1);
	ranges2.push_back(range);
	rootSignFactory.AddDescriptorTable(ranges2); //4 //all mesh materials

	state->RootSignature = rootSignFactory.CreateSignture(context->Device.Get());

	PipelineStateFactory pipeStateFact;
	//depth state
	D3D12_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

	pipeStateFact.SetDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	pipeStateFact.SetDepthStencilState(depthDesc);
	pipeStateFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipeStateFact.SetInputLayout(VertexLayout, VertexLayoutSize);
	pipeStateFact.SetRootSignature(state->RootSignature.Get());
	std::vector<DXGI_FORMAT> formats;
	formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipeStateFact.SetRenderTargetFormats(formats);
	pipeStateFact.SetAllShaders(state->Shader);
	state->PipelineState = pipeStateFact.CreateGraphicsState(context);

	state->DiffSkyTex.Init("assets/cubemaps/skybox_irr.dds", context->Device.Get(), cmdList);
	state->SpecSkyTex.Init("assets/cubemaps/skybox_rad.dds", context->Device.Get(), cmdList);
	state->IBLTex.Init("assets/textures/IBLTex.dds", context->Device.Get(), cmdList);

	D3D12_DESCRIPTOR_HEAP_DESC skyHeapDesc = {};
	skyHeapDesc.NumDescriptors = ENVIRONMENT_MATERIAL_SIZE;
	skyHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	skyHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(context->Device->CreateDescriptorHeap(&skyHeapDesc, IID_PPV_ARGS(&state->SkyDescHeap)), L"Error creating descriptor heap for SkyTex");

	CD3DX12_CPU_DESCRIPTOR_HANDLE skyHandle(state->SkyDescHeap->GetCPUDescriptorHandleForHeapStart());

	state->DiffSkyTex.CreateSRV(context->Device.Get(), skyHandle);
	state->SpecSkyTex.CreateSRV(context->Device.Get(), skyHandle.Offset(1, state->DescHeapIncSize));
	state->IBLTex.CreateSRV(context->Device.Get(), skyHandle.Offset(1, state->DescHeapIncSize));

	D3D12_DESCRIPTOR_HEAP_DESC renderHeapDesc = {};
	renderHeapDesc.NumDescriptors = MAX_RENDER_OBJECTS * MATERIAL_SIZE + ENVIRONMENT_MATERIAL_SIZE;
	renderHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	renderHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(context->Device->CreateDescriptorHeap(&renderHeapDesc, IID_PPV_ARGS(&state->RenderDescHeap)), L"Error creating descriptor heap for Rendering");
	//copy enivroment descriptors
	context->Device->CopyDescriptorsSimple(ENVIRONMENT_MATERIAL_SIZE, state->RenderDescHeap->GetCPUDescriptorHandleForHeapStart(),
		state->SkyDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_INDIRECT_ARGUMENT_DESC argsDesc[3];
	//draw ID
	argsDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argsDesc[0].Constant.RootParameterIndex = DRAW_INDEX_CONSTANT;
	argsDesc[0].Constant.Num32BitValuesToSet = 1;
	argsDesc[0].Constant.DestOffsetIn32BitValues = 0;
	//material ID
	argsDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argsDesc[1].Constant.RootParameterIndex = DRAW_INDEX_CONSTANT;
	argsDesc[1].Constant.Num32BitValuesToSet = 1;
	argsDesc[1].Constant.DestOffsetIn32BitValues = 1;
	//draw args
	argsDesc[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC signDesc;
	signDesc.ByteStride = sizeof(IndirectDrawCall);
	signDesc.NumArgumentDescs = 3;
	signDesc.pArgumentDescs = argsDesc;
	signDesc.NodeMask = 0;
	HR(context->Device->CreateCommandSignature(&signDesc, state->RootSignature.Get(), IID_PPV_ARGS(&state->CommandSignature)),L"Error creating Command signature");

#ifdef _DEBUG
	state->RenderDescHeap->SetName(L"RenderDescHeap");
	state->SkyDescHeap->SetName(L"SkyDescHeap");
#endif

}

void RenderGeometryWithoutCulling(ID3D12GraphicsCommandList* cmdList, GeometryProgramState* state, RenderQueue* queue) {
	cmdList->SetGraphicsRootSignature(state->RootSignature.Get());
	cmdList->SetPipelineState(state->PipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(PER_FRAME_CONST_BUFFER, g_BufferManager.GetGPUHandle("cbPerFrame2"));
	cmdList->SetGraphicsRootShaderResourceView(SHADER_INPUT_STRUCT_BUFFER, g_BufferManager.GetGPUHandle("ShaderInputBuffer"));
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_ModelBank.ApplyVertexBuffers(cmdList);

	ID3D12DescriptorHeap* heaps[] = { state->RenderDescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	//set enviroment
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(state->RenderDescHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootDescriptorTable(ENVIROMENT_DESC_TABLE, gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(MATERIAL_DESC_TABLE, gpuHandle.Offset(ENVIRONMENT_MATERIAL_SIZE * state->DescHeapIncSize));

	g_BufferManager.SwitchState(cmdList, "IndirectBuffer", D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	g_ModelBank.ApplyIndexBuffers(cmdList);
	cmdList->ExecuteIndirect(state->CommandSignature.Get(), queue->GetDrawCount(),
		g_BufferManager.GetBufferResource("IndirectBuffer"), 0, nullptr, 0);
}

void RenderGeometry(ID3D12GraphicsCommandList*cmdList, GeometryProgram::GeometryProgramState* state, FilterContext* filterContext, DrawCullingProgram* cullingProgram) {
	filterContext->BeginRender(cmdList);
	cmdList->SetGraphicsRootSignature(state->RootSignature.Get());
	cmdList->SetPipelineState(state->PipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(PER_FRAME_CONST_BUFFER, g_BufferManager.GetGPUHandle("cbPerFrame2"));
	cmdList->SetGraphicsRootShaderResourceView(SHADER_INPUT_STRUCT_BUFFER, g_BufferManager.GetGPUHandle("ShaderInputBuffer"));
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_ModelBank.ApplyVertexBuffers(cmdList);

	ID3D12DescriptorHeap* heaps[] = { state->RenderDescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	//set enviroment
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(state->RenderDescHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootDescriptorTable(ENVIROMENT_DESC_TABLE, gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(MATERIAL_DESC_TABLE, gpuHandle.Offset(ENVIRONMENT_MATERIAL_SIZE * state->DescHeapIncSize));

	CD3DX12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(filterContext->GetDrawArgsResource(filterContext->GetRenderIndex()),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT),
		CD3DX12_RESOURCE_BARRIER::Transition(cullingProgram->GetCounterBuffer(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)
	};

	cmdList->ResourceBarrier(_countof(barriers), barriers);

	//cmdList->ExecuteIndirect(state->CommandSignature.Get(), filterContext->GetBatchCount(),
	//	filterContext->GetDrawArgsResource(filterContext->GetRenderIndex()), 0,
	//	nullptr, 0);
	cmdList->ExecuteIndirect(state->CommandSignature.Get(), filterContext->GetBatchCount(),
		cullingProgram->GetDrawArgsBuffer(filterContext->GetRenderIndex()), 0,
		cullingProgram->GetCounterBuffer(), filterContext->GetCounterOffset());
}