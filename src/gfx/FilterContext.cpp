#include "FilterContext.h"

FilterContext::FilterContext() {

}

FilterContext::~FilterContext() {

}

void FilterContext::Init(DX12Context* context) {
	//create resources
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MAX_INDEX_BUFFER_SIZE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	CD3DX12_RESOURCE_DESC drawBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MAX_DRAW_ARGS_BUFFER_SIZE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < MAX_SIMUL_PASSES; i++) {
		context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_IndexBuffers[i]));
		context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&drawBufferDesc, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, nullptr, IID_PPV_ARGS(&m_DrawArgsBuffers[i]));
	}
}

void FilterContext::Clear() {
	m_CurrentBatch = 0;
	m_CurrentFilterIndex = 0;
	m_CurrentRenderIndex = 0;
}

void FilterContext::BeginFilter(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut, ID3D12Resource** indexBufferOut) {
	m_CurrentFilterIndex = ++m_CurrentFilterIndex % 2;

	*drawArgsOut = m_DrawArgsBuffers[m_CurrentFilterIndex].Get();
	*indexBufferOut = m_IndexBuffers[m_CurrentFilterIndex].Get();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentFilterIndex].Get(),
		D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void FilterContext::BeginRender(ID3D12GraphicsCommandList* cmdList, ID3D12Resource** drawArgsOut, ID3D12Resource** indexBufferOut) {
	m_CurrentRenderIndex = ++m_CurrentRenderIndex % 2;

	*drawArgsOut = m_DrawArgsBuffers[m_CurrentRenderIndex].Get();
	*indexBufferOut = m_IndexBuffers[m_CurrentRenderIndex].Get();

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DrawArgsBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffers[m_CurrentRenderIndex].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDEX_BUFFER));
}