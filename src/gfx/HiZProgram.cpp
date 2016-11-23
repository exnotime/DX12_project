#include "HiZProgram.h"
#include "RootSignatureFactory.h"
HiZProgram::HiZProgram() {

}

HiZProgram::~HiZProgram() {

}

void HiZProgram::Init(DX12Context* context, glm::vec2 screenSize) {
	m_Shader.LoadFromFile(L"src/shaders/HiZGeneration.hlsl", COMPUTE_SHADER_BIT, nullptr);

	m_ScreenSize = screenSize * 0.5f;
	m_MipCount = log2(glm::max(m_ScreenSize.x, m_ScreenSize.y));
	m_DescIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//hi-z
	RootSignatureFactory rootSignFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE range = {};
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0); //0
	ranges.push_back(range);
	rootSignFact.AddDescriptorTable(ranges);
	rootSignFact.AddConstant(2, 0);
	m_RootSignature = rootSignFact.CreateSignture(context->Device.Get());

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipeDesc = {};
	pipeDesc.CS = m_Shader.GetByteCode(COMPUTE_SHADER_BIT);
	pipeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipeDesc.NodeMask = 0;
	pipeDesc.pRootSignature = m_RootSignature.Get();

	HR(context->Device->CreateComputePipelineState(&pipeDesc, IID_PPV_ARGS(&m_PipelineState)), L"Error creating Compute pipeline state: HIZ");

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.NumDescriptors = m_MipCount;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_DescHeap));

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.MipLevels = m_MipCount;
	resourceDesc.Width = m_ScreenSize.x;
	resourceDesc.Height = m_ScreenSize.y;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_HiZResource));
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.PlaneSlice = 0;

	for (int i = 0; i < m_MipCount; i++) {
		uavDesc.Texture2D.MipSlice = i;
		context->Device->CreateUnorderedAccessView(m_HiZResource.Get(), nullptr, &uavDesc, uavHandle);
		uavHandle.Offset(1, m_DescIncSize);
	}
	//linearize depth
	m_LinearizeShader.LoadFromFile(L"src/shaders/CopyDepth.hlsl", COMPUTE_SHADER_BIT);

	RootSignatureFactory linRootsignFact;
	ranges.clear();
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges.push_back(range);
	linRootsignFact.AddDescriptorTable(ranges); //0

	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges2;
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	ranges2.push_back(range);
	linRootsignFact.AddDescriptorTable(ranges2); //1

	linRootsignFact.AddConstant(2, 0);
	m_LinearRoot = linRootsignFact.CreateSignture(context->Device.Get());

	pipeDesc = {};
	pipeDesc.CS = m_LinearizeShader.GetByteCode(COMPUTE_SHADER_BIT);
	pipeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipeDesc.NodeMask = 0;
	pipeDesc.pRootSignature = m_LinearRoot.Get();

	HR(context->Device->CreateComputePipelineState(&pipeDesc, IID_PPV_ARGS(&m_LinearPipe)), L"Error creating Compute pipeline state: Linearize Depth");

	D3D12_DESCRIPTOR_HEAP_DESC linearHeapDesc = {};
	linearHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	linearHeapDesc.NumDescriptors = 2;
	linearHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context->Device->CreateDescriptorHeap(&linearHeapDesc, IID_PPV_ARGS(&m_LinearHeap));
	
}

void HiZProgram::CreateDescriptors(ID3D12Device* device, ID3D12Resource* srcTex) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_LinearHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateShaderResourceView(srcTex, &srvDesc, descHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.PlaneSlice = 0;
	uavDesc.Texture2D.MipSlice = 0;
	device->CreateUnorderedAccessView(m_HiZResource.Get(), nullptr, &uavDesc, descHandle.Offset(1, m_DescIncSize));
}

void HiZProgram::Disbatch(ID3D12GraphicsCommandList* cmdList) {
	UINT workGroupSize = 16;
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_HiZResource.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_LinearHeap->GetGPUDescriptorHandleForHeapStart());
	//linearize the depth buffer
	cmdList->SetPipelineState(m_LinearPipe.Get());
	cmdList->SetComputeRootSignature(m_LinearRoot.Get());
	ID3D12DescriptorHeap* linheaps[] = { m_LinearHeap.Get() };
	cmdList->SetDescriptorHeaps(1, linheaps);
	cmdList->SetComputeRootDescriptorTable(0, handle);
	cmdList->SetComputeRootDescriptorTable(1, handle.Offset(1, m_DescIncSize));
	cmdList->SetComputeRoot32BitConstants(2, 2, &m_ScreenSize[0], 0);

	UINT x = (m_ScreenSize.x + workGroupSize - 1) / workGroupSize;
	UINT y = (m_ScreenSize.y + workGroupSize - 1) / workGroupSize;
	cmdList->Dispatch(x, y, 1);

	//hi-z
	handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_DescHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->SetComputeRootSignature(m_RootSignature.Get());
	ID3D12DescriptorHeap* heaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);

	glm::ivec2 targetsize = m_ScreenSize;
	for (int i = 0; i < m_MipCount - 1; ++i) {
		cmdList->SetComputeRootDescriptorTable(0, handle);
		cmdList->SetComputeRoot32BitConstants(1, 2, &targetsize[0], 0);
		targetsize /= 2;
		x = (targetsize.x + workGroupSize - 1) / workGroupSize;
		y = (targetsize.y + workGroupSize - 1) / workGroupSize;

		

		cmdList->Dispatch(x, y, 1);
		handle.Offset(1, m_DescIncSize);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_HiZResource.Get()));
	}
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_HiZResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}