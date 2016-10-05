#pragma once
#include "DX12Common.h"
#include <unordered_map>
#define g_BufferManager BufferManager::GetInstance()

enum BUFFER_TYPE {
	CONST_BUFFER,
	STRUCTURED_BUFFER,
	INDIRECT_BUFFER,
	RAW_BUFFER
};

struct BufferInfo {
	bool DoubleBuffered = false;
	bool GPUOnly = false;
	BUFFER_TYPE Type;
	UINT SizeInBytes = 0;
	UINT StructSize;
	void* Data = nullptr;
};

class BufferManager {
public:
	BufferManager();
	static BufferManager& GetInstance();
	
	void Init(DX12Context* context, UINT maxBuffers = 100);

	void CreateConstBuffer( const std::string& name, void* data, UINT size);
	void CreateIndirectBuffer(const std::string& name, void* data, UINT size);
	void CreateStructuredBuffer( const std::string& name, void* data, UINT size, UINT structureSize, bool doubleBuffered = false, bool GPUOnly = false);

	void CreateBuffer(const std::string& name, const BufferInfo& info);

	void UpdateBuffer(const std::string& name, void* data, UINT size);
	void* MapBuffer(const std::string& name);
	void UnMapBuffer(const std::string& name);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(const std::string& name);
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUHandle(const std::string& name);
	ID3D12Resource* GetBufferResource(const std::string& name);
	void SwitchState(const std::string& name, D3D12_RESOURCE_STATES state);
private:
	~BufferManager();

	struct Buffer {
		ComPtr<ID3D12Resource> Resource[g_FrameCount];
		ComPtr<ID3D12Resource> UploadHeap;
		UINT Offset;
		BUFFER_TYPE Type;
		D3D12_RESOURCE_STATES State;
		bool DoubleBuffered;
		bool GPUOnly;
	};

	DX12Context* m_Context;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	std::unordered_map<std::string, Buffer*> m_Buffers;
	UINT m_BufferCounter = 0;
	UINT m_HeapIncrementSize = 0;
};