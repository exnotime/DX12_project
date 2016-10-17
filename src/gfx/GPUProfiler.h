#pragma once
#include "DX12Common.h"
#include <stdio.h>
//#define SILENT_PROFILING
#define PRINT_TO_FILE
#define MAX_PROFILER_STEPS 128
class GPUProfiler{
public:
	GPUProfiler();
	~GPUProfiler();
	void Init(DX12Context* context);
	void Start();
	void Step(ID3D12GraphicsCommandList* cmdList, const std::string& name);
	void End(ID3D12GraphicsCommandList* cmdList);
	void PrintResults();
private:
	UINT64 m_TimerFreqs;
	UINT m_StepCounter;
	bool m_Started;
	ComPtr<ID3D12QueryHeap> m_QueryHeap;
	ComPtr<ID3D12Resource> m_ResultBuffer;
	std::vector<std::string> m_StepNames;
#ifdef PRINT_TO_FILE
	FILE* m_File;
#endif
};