#pragma once
#include "DX12Common.h"
enum COMMAND_BUFFER_TYPE {
	GRAPHICS_TYPE,
	COMPUTE_TYPE,
	COPY_TYPE,
	TYPE_COUNT
};

class CommandBuffer {
public:
	CommandBuffer();
	~CommandBuffer();
	void Init(ID3D12Device* device, COMMAND_BUFFER_TYPE type, UINT cmdListCount);
	void ResetAllocator(int frameIndex);
	void ResetCommandLists(int frameIndex);
	ID3D12GraphicsCommandList* GetNextCmdList();
	void Execute(ID3D12CommandQueue* queue);

private:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocators[g_FrameCount];
	std::vector<ComPtr<ID3D12GraphicsCommandList>> m_CmdLists;
	std::vector<ID3D12GraphicsCommandList*> m_ClosedCmdLists;
	UINT m_Numerator = 0;
};