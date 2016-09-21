#pragma once
#include "Common.h"
//#define SILENT_PROFILING
#define MAX_PROFILER_STEPS 128
class GPUProfiler{
public:
	GPUProfiler();
	~GPUProfiler();
	void Init(DX12Context* context);
	void Step(ID3D12GraphicsCommandList* cmdList);
	void End(ID3D12GraphicsCommandList* cmdList);
	void PrintResults();
private:
	UINT64 m_TimerFreqs;
	UINT m_StepCounter;
	bool m_Started;
	ComPtr<ID3D12QueryHeap> m_QueryHeap;
	ComPtr<ID3D12Resource> m_ResultBuffer;
};