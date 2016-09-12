#pragma once
#include "Common.h"
class GPUProfiler{
public:
	GPUProfiler();
	~GPUProfiler();
	void Init(DX12Context* context);
	void Start(ID3D12GraphicsCommandList* cmdList);
	void End(ID3D12GraphicsCommandList* cmdList);
	double GetResults();
private:
	UINT64 m_TimerFreqs;
	UINT64 m_Start, m_End;
	bool m_Started;
	ComPtr<ID3D12QueryHeap> m_QueryHeap;
	ComPtr<ID3D12Resource> m_ResultBuffer;
};