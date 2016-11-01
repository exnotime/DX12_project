#include "GPUProfiler.h"
#include <stdio.h>
#include "TestParams.h"
GPUProfiler::GPUProfiler() {

}
GPUProfiler::~GPUProfiler() {
#ifdef PRINT_TO_FILE
	fclose(m_File);
#endif
}
void GPUProfiler::Init(DX12Context* context) {
	D3D12_QUERY_HEAP_DESC queryHeapDesc;
	queryHeapDesc.Count = MAX_PROFILER_STEPS * g_FrameCount;
	queryHeapDesc.NodeMask = 0;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	context->Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_QueryHeap));

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(MAX_PROFILER_STEPS * g_FrameCount * sizeof(UINT64)), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_ResultBuffer));
	m_TimerFreqs = 0;
	context->Device->SetStablePowerState(true);
	context->CommandQueue->GetTimestampFrequency(&m_TimerFreqs);

#ifdef PRINT_TO_FILE
	m_File = fopen(g_TestParams.Filename.c_str(), "w");
#endif
	m_StepCounter = 0;
	m_LastFrameSteps = 1;

}
void GPUProfiler::Start() {
	m_StepNames.clear();
	m_StepCounter = 0;
}

void GPUProfiler::Step(ID3D12GraphicsCommandList* cmdList, const std::string& name) {
		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, m_StepCounter++);
#ifndef SILENT_PROFILING
		m_StepNames.push_back(name);
#endif
}

void GPUProfiler::End(ID3D12GraphicsCommandList* cmdList, UINT frameIndex) {
		cmdList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, m_StepCounter);
		cmdList->ResolveQueryData(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, ++m_StepCounter, m_ResultBuffer.Get(), MAX_PROFILER_STEPS * frameIndex * sizeof(UINT64));
}

void GPUProfiler::PrintResults(UINT frameIndex) {
	if (m_LastFrameSteps == 0)
		return;
	UINT64* result;
	UINT index = MAX_PROFILER_STEPS * frameIndex;

	D3D12_RANGE range = { index * sizeof(UINT64),index * sizeof(UINT64) + sizeof(UINT64) * m_LastFrameSteps };
	m_ResultBuffer->Map(0, &range, (void**)&result);
	
	UINT64 a, b;
	for (int i = 0; i < m_LastFrameSteps - 1; i++) {
 		a = result[index + i];
		b = result[index + i + 1];

		double res = ((b - a) / (double)m_TimerFreqs) * 1000.0;
#ifndef SILENT_PROFILING
		printf("Step %s: %2.3f ms\n", m_LastFrameNames[i].c_str(), res);
#endif
	}

	a = result[index];
	b = result[index + m_LastFrameSteps - 1];

	UINT64 delta = b - a;
	double res = (delta / (double)m_TimerFreqs) * 1000.0;

#ifndef SILENT_PROFILING
	printf("Entire frame: %4.4f ms\n", res);
#endif

#ifdef PRINT_TO_FILE
	m_LogCounter++;
	fprintf(m_File, "%d %f\n", m_LogCounter, res);
#endif

	range.End = 0;
	m_ResultBuffer->Unmap(0, &range);
	m_LastFrameSteps = m_StepCounter;
	m_LastFrameNames = m_StepNames;
}

void GPUProfiler::Reset() {
#ifdef PRINT_TO_FILE
	fclose(m_File);
	m_File = fopen(g_TestParams.Filename.c_str(), "w");
	m_LogCounter = 0;
#endif
	m_LastFrameSteps = 1;
	m_LastFrameNames.clear();
	m_StepCounter = 0;
	m_StepNames.clear();
}