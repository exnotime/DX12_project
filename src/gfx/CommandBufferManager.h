#pragma once
#include "CommandBuffer.h"
#define g_CommandBufferManager CommandBufferManager::GetInstance()

class CommandBufferManager{
public:
	~CommandBufferManager();
	static CommandBufferManager& GetInstance();
	void Init(DX12Context* context, UINT graphicsCount = 8, UINT computeCount = 2, UINT copyCount = 1);

	ID3D12GraphicsCommandList* GetNextCommandList(COMMAND_BUFFER_TYPE type);
	void ResetAllCommandBuffers();
	void ExecuteCommandBuffers(COMMAND_BUFFER_TYPE type);

	void SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE sender, COMMAND_BUFFER_TYPE reciever); //what queue should signal what fence
	void WaitOnFenceSignal(UINT64 signal, COMMAND_BUFFER_TYPE waiter); //what queue should wait on its fence
private:
	CommandBufferManager();

	DX12Context* m_Context;
	CommandBuffer m_CommandBuffers[TYPE_COUNT];
	ComPtr<ID3D12Fence> m_Fences[TYPE_COUNT];
};