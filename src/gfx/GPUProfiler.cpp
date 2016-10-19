#include "GPUProfiler.h"
#include <stdio.h>
GPUProfiler::GPUProfiler() {

}
GPUProfiler::~GPUProfiler() {
#ifdef PRINT_TO_FILE
	fclose(m_File);
#endif
}
void GPUProfiler::Init(DX12Context* context) {
	D3D12_QUERY_HEAP_DESC queryHeapDesc;
	queryHeapDesc.Count = MAX_PROFILER_STEPS;
	queryHeapDesc.NodeMask = 0;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	context->Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_QueryHeap));

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(MAX_PROFILER_STEPS * sizeof(UINT64)), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_ResultBuffer));
	m_TimerFreqs = 0;
	context->Device->SetStablePowerState(true);
	context->CommandQueue->GetTimestampFrequency(&m_TimerFreqs);

#ifdef PRINT_TO_FILE
	m_File = fopen("ProfilerLog.txt", "a");
#endif
	m_StepCounter = 0;

}
void GPUProfiler::Start() {
	m_StepNames.clear();
	m_StepCounter = 0;
}

void GPUProfiler::Step(ID3D12GraphicsCommandList* cmdList, const std::string& name) {
		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, m_StepCounter++);
		m_StepNames.push_back(name);
}
void GPUProfiler::End(ID3D12GraphicsCommandList* cmdList) {
		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, m_StepCounter);
		cmdList->ResolveQueryData(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, ++m_StepCounter, m_ResultBuffer.Get(), 0);
}

void GPUProfiler::PrintResults() {
	if (m_StepCounter == 0)
		return;

	UINT64* result;
	D3D12_RANGE range = { 0, sizeof(UINT64) * m_StepCounter };
	m_ResultBuffer->Map(0, &range, (void**)&result);
	static int counter = 0;
	UINT64 a, b;
	for (int i = 0; i < m_StepCounter - 1; i++) {
		a = result[i];
		b = result[i + 1];

		double res = ((b - a) / (double)m_TimerFreqs) * 1000.0;
#ifndef SILENT_PROFILING
		printf("Step %s: %2.3f ms\n", m_StepNames[i].c_str(), res);
#endif
#ifdef PRINT_TO_FILE
		fprintf(m_File, "%d %2.3f\n", counter, res);
#endif
	}
	counter++;
	a = result[0];
	b = result[m_StepCounter - 1];

	double res = ((b - a) / (double)m_TimerFreqs) * 1000.0;
	printf("Entire frame: %4.4f ms\n", res);



	range.End = 0;
	m_ResultBuffer->Unmap(0, &range);
}