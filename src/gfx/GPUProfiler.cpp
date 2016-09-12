#include "GPUProfiler.h"

GPUProfiler::GPUProfiler() {

}
GPUProfiler::~GPUProfiler() {

}
void GPUProfiler::Init(DX12Context* context) {
	D3D12_QUERY_HEAP_DESC queryHeapDesc;
	queryHeapDesc.Count = 2;
	queryHeapDesc.NodeMask = 0;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	context->Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_QueryHeap));

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(256), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_ResultBuffer));
	m_TimerFreqs = 0;
	context->Device->SetStablePowerState(true);
	context->CommandQueue->GetTimestampFrequency(&m_TimerFreqs);

	m_Started = false;
}
void GPUProfiler::Start(ID3D12GraphicsCommandList* cmdList) {
	if (!m_Started) {
		//cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ResultBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
		m_Started = true;
	}
}
void GPUProfiler::End(ID3D12GraphicsCommandList* cmdList) {
	if (m_Started) {
		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
		cmdList->ResolveQueryData(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, m_ResultBuffer.Get(), 0);
		//cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ResultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		m_Started = false;
	}
}

double GPUProfiler::GetResults() {
	UINT64* result;
	D3D12_RANGE range = {0, sizeof(UINT64) * 2};
	m_ResultBuffer->Map(0, &range, (void**)&result);

	m_Start = result[0];
	m_End = result[1];
	double res = ((m_End - m_Start) / (double)m_TimerFreqs) * 1000.0;

	range.End = 0;
	m_ResultBuffer->Unmap(0, &range);

	return res;
}