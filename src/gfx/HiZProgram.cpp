#include "HiZProgram.h"
#include "RootSignatureFactory.h"
HiZProgram::HiZProgram() {

}

HiZProgram::~HiZProgram() {

}

void HiZProgram::Init(DX12Context* context, glm::vec2 screenSize) {
	m_Shader.LoadFromFile(L"src/shaders/HiZGeneration.hlsl", COMPUTE_SHADER_BIT);

	m_ScreenSize = screenSize * 0.5f;
	m_MipCount = log2(glm::max(m_ScreenSize.x, m_ScreenSize.y));
	m_DescIncSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	RootSignatureFactory rootSignFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE range = {};
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0); //0
	ranges.push_back(range);
	rootSignFact.AddDescriptorTable(ranges);

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
	resourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_HiZResource));
	
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
}

void HiZProgram::Disbatch(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* srcTex) {
	//copy from the src texture to mip 0
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srcTex, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_HiZResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));

	D3D12_TEXTURE_COPY_LOCATION dst, src;
	dst.pResource = m_HiZResource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	src = dst;
	src.pResource = srcTex;
	cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srcTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_HiZResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


	CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_DescHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->SetComputeRootSignature(m_RootSignature.Get());
	ID3D12DescriptorHeap* heaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, heaps);

	glm::uvec2 targetsize = m_ScreenSize;
	UINT workGroupSize = 8;

	for (int i = 0; i < m_MipCount - 1; ++i) {
		cmdList->SetComputeRootDescriptorTable(0, uavHandle);
		targetsize /= 2;
		UINT x = (targetsize.x + workGroupSize - 1) / workGroupSize;
		UINT y = (targetsize.y + workGroupSize - 1) / workGroupSize;

		cmdList->Dispatch(x, y, 1);
		uavHandle.Offset(1, m_DescIncSize);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_HiZResource.Get()));
	}
}