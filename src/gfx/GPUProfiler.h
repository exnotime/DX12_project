#pragma once
#include "DX12Common.h"
#include "TestParams.h"
#include <stdio.h>
#define SILENT_LOG
#define MAX_PROFILER_STEPS 8192
class GPUProfiler{
public:
	GPUProfiler();
	~GPUProfiler();
	void Init(DX12Context* context);
	void Start();
	void Step(ID3D12GraphicsCommandList* cmdList, const std::string& name);
	void End(ID3D12GraphicsCommandList* cmdList, UINT frameIndex);
	void PrintResults(UINT frameIndex);
	void Reset();
private:
	UINT64 m_TimerFreqs;
	UINT m_StepCounter;
	UINT m_LastFrameSteps;
	bool m_Started;
	ComPtr<ID3D12QueryHeap> m_QueryHeap;
	ComPtr<ID3D12Resource> m_ResultBuffer;
	std::vector<std::string> m_StepNames;
	std::vector<std::string> m_LastFrameNames;
#ifdef PRINT_TO_FILE
	int m_LogCounter = 0;
	FILE* m_File;
#endif
};