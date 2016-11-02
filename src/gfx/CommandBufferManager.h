#pragma once
#include "CommandBuffer.h"
#define g_CommandBufferManager CommandBufferManager::GetInstance()

class CommandBufferManager{
public:
	~CommandBufferManager();
	static CommandBufferManager& GetInstance();
	void Init(DX12Context* context, UINT graphicsCount = 8, UINT computeCount = 2, UINT copyCount = 1);

	CommandBuffer* GetNextCommandBuffer(COMMAND_BUFFER_TYPE type);
	void ResetAllCommandBuffers();
	void ExecuteCommandBuffers(const std::vector<CommandBuffer*>& buffers, COMMAND_BUFFER_TYPE type);
	void ExecuteCommandBuffer(CommandBuffer* buffer, COMMAND_BUFFER_TYPE type);

	void SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE sender, COMMAND_BUFFER_TYPE reciever); //what queue should signal what fence
	void SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE receiver);
	void WaitOnFenceSignal(UINT64 signal, COMMAND_BUFFER_TYPE waiter); //what queue should wait on its fence
private:
	CommandBufferManager();

	DX12Context* m_Context;
	std::vector<CommandBuffer> m_GraphicsBuffers;
	std::vector<CommandBuffer> m_ComputeBuffers;
	std::vector<CommandBuffer> m_CopyBuffers;

	UINT m_BufferCounters[CMD_BUFFER_TYPE_COUNT];
	ComPtr<ID3D12Fence> m_Fences[CMD_BUFFER_TYPE_COUNT];
};