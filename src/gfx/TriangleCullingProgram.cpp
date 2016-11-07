#include "TriangleCullingProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "BufferManager.h"
#include "ModelBank.h"
#include "TestParams.h"
#include <core/Timer.h>
#include <sstream>
TriangleCullingProgram::TriangleCullingProgram() {

}

TriangleCullingProgram::~TriangleCullingProgram() {
}

void TriangleCullingProgram::Init(DX12Context* context) {
	m_Context = context;
	//build shader macros
	std::vector<D3D_SHADER_MACRO> macros;
	if (g_TestParams.Instrument) {
		macros.push_back({ "INSTRUMENT","1" });
	}
	D3D_SHADER_MACRO macro;
	macro.Name = "BATCH_SIZE";
	std::string s;
	std::stringstream ss;
	ss << g_TestParams.CurrentTest.BatchSize;
	s = ss.str();
	macro.Definition = s.c_str();
	macros.push_back(macro);
	
	m_Shader.LoadFromFile(L"src/shaders/TriangleCulling.hlsl", COMPUTE_SHADER_BIT, &context->Extensions, &macros);
	//Root sign
	RootSignatureFactory rootSignFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> inputRanges;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> outputRanges;
	for (int i = 0; i < ROOT_PARAM_COUNT; ++i) {
		switch (i)
		{
		case PER_FRAME_CB:
			rootSignFact.AddConstantBufferView(0);
			break;
		case CONSTANTS_C:
			rootSignFact.AddConstant(3, 1);
			break;
		case INPUT_DT:
			inputRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0));
			rootSignFact.AddDescriptorTable(inputRanges);
			break;
		case OUTPUT_DT:
			outputRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0, 0, 0));
			rootSignFact.AddDescriptorTable(outputRanges);
			break;
		}
	}
	rootSignFact.AddExtensions(&context->Extensions);
	D3D12_STATIC_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.MipLODBias = 0;
	sampDesc.MaxAnisotropy = 1.0f;
	sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampDesc.MinLOD = 0.0f;
	sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	sampDesc.ShaderRegister = 0;
	sampDesc.RegisterSpace = 0;
	sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootSignFact.AddStaticSampler(sampDesc);
	m_RootSign = rootSignFact.CreateSignture(context->Device.Get());
	//Pipe state
	PipelineStateFactory pipeFact;
	pipeFact.SetRootSignature(m_RootSign.Get());
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	m_PipeState = pipeFact.CreateComputeState(context);

	//Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 11;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeap));
	m_DescHeapIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void TriangleCullingProgram::Reset(DX12Context* context) {
	//recompile shader and pipeline state
	m_PipeState.Reset();
	std::vector<D3D_SHADER_MACRO> macros;
	if (g_TestParams.Instrument) {
		macros.push_back({ "INSTRUMENT","1" });
	}
	D3D_SHADER_MACRO macro;
	macro.Name = "BATCH_SIZE";
	std::string s;
	std::stringstream ss;
	ss << g_TestParams.CurrentTest.BatchSize;
	s = ss.str();
	macro.Definition = s.c_str();
	macros.push_back(macro);

	m_Shader.LoadFromFile(L"src/shaders/TriangleCulling.hlsl", COMPUTE_SHADER_BIT, &context->Extensions, &macros);

	PipelineStateFactory pipeFact;
	pipeFact.SetRootSignature(m_RootSign.Get());
	pipeFact.SetShader(m_Shader.GetByteCode(COMPUTE_SHADER_BIT), COMPUTE_SHADER_BIT);
	m_PipeState = pipeFact.CreateComputeState(context);
}

void TriangleCullingProgram::CreateDescriptorTable(HiZProgram* hizProgram) {
	ID3D12Device* device =  m_Context->Device.Get();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//draw args input
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = 10000;
	srvDesc.Buffer.StructureByteStride = sizeof(IndirectDrawCall);
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	device->CreateShaderResourceView(g_BufferManager.GetBufferResource("IndirectBuffer"), &srvDesc, cpuHandle);
	//vertices
	srvDesc.Buffer.NumElements = g_ModelBank.GetVertexCount();
	srvDesc.Buffer.StructureByteStride = sizeof(glm::vec3);
	device->CreateShaderResourceView(g_ModelBank.GetVertexBufferResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//indices
	srvDesc.Buffer.NumElements = g_ModelBank.GetIndexCount();
	srvDesc.Buffer.StructureByteStride = sizeof(UINT);
	device->CreateShaderResourceView(g_ModelBank.GetIndexBufferResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//shader inputs
	srvDesc.Buffer.NumElements = 10000;
	srvDesc.Buffer.StructureByteStride = sizeof(ShaderInput);
	device->CreateShaderResourceView(g_BufferManager.GetBufferResource("ShaderInputBuffer"), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));
	//hi-z
	srvDesc.Texture2D.MipLevels = hizProgram->GetMipCount();
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	device->CreateShaderResourceView(hizProgram->GetResource(), &srvDesc, cpuHandle.Offset(1, m_DescHeapIncSize));

}

bool TriangleCullingProgram::Disbatch(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue, FilterContext* filterContext) {
	filterContext->BeginFilter(cmdList);
	// copy in descriptors
	m_Context->Device->CopyDescriptorsSimple(3, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart(), m_DescHeapIncSize * (5 + 3 * filterContext->GetFilterIndex())),
		filterContext->GetFilterDescriptors(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ID3D12DescriptorHeap* heaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->SetComputeRootSignature(m_RootSign.Get());
	cmdList->SetPipelineState(m_PipeState.Get());
	cmdList->SetComputeRootConstantBufferView(PER_FRAME_CB, g_BufferManager.GetGPUHandle("cbPerFrame"));
	cmdList->SetComputeRootDescriptorTable(INPUT_DT, m_DescHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->SetComputeRootDescriptorTable(OUTPUT_DT, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_DescHeap->GetGPUDescriptorHandleForHeapStart(), m_DescHeapIncSize * ( 5 + 3 * filterContext->GetFilterIndex())));

	UINT batchCounter = 0;
	for (int i = filterContext->GetCurrentDraw(); i < queue->GetDrawCount(); i++) {

		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, batchCounter, 0); //batch id
		cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, i, 1); // draw id

		UINT batchCount = ((queue->GetDrawList()[i].DrawArgs.IndexCountPerInstance / 3) + g_TestParams.CurrentTest.BatchSize - 1) / g_TestParams.CurrentTest.BatchSize;

		if(filterContext->GetRemainder() > 0)
			cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, (batchCount - filterContext->GetRemainder()) * g_TestParams.CurrentTest.BatchSize * 3, 2); // batch offset
		else
			cmdList->SetComputeRoot32BitConstant(CONSTANTS_C, 0, 2);

		//if we have filled up on batches break and switch to rendering
		if (filterContext->AddBatches(batchCount, batchCount)) {
			cmdList->Dispatch(batchCount, 1, 1);
			return true;
		}
		cmdList->Dispatch(batchCount, 1, 1);
		filterContext->IncrementDrawCounter();
		batchCounter += batchCount;
	}

	return false;
}
