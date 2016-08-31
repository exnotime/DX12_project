#pragma once
#include "Common.h"
#include <unordered_map>
#define g_BufferManager BufferManager::GetInstance()

class BufferManager {
public:
	BufferManager();
	static BufferManager& GetInstance();
	
	void Init(DX12Context* context, UINT maxBuffers = 10);
	void CreateConstBuffer( const std::string& name, void* data, UINT size);
	void CreateStructuredBuffer( const std::string& name, void* data, UINT size, UINT structureSize);
	void UpdateBuffer(const std::string& name, void* data, UINT size);
	void* MapBuffer(const std::string& name);
	void UnMapBuffer(const std::string& name);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(const std::string& name);
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUHandle(const std::string& name);
private:
	~BufferManager();

	struct Buffer {
		ComPtr<ID3D12Resource> Resource;
		ComPtr<ID3D12Resource> UploadHeap;
		UINT Offset;
	};

	DX12Context* m_Context;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	std::unordered_map<std::string, Buffer*> m_Buffers;
	UINT m_BufferCounter = 0;
	UINT m_HeapIncrementSize = 0;
};