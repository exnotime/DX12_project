#include "CommandBufferManager.h"

CommandBufferManager::CommandBufferManager() {

}

CommandBufferManager::~CommandBufferManager() {

}

CommandBufferManager& CommandBufferManager::GetInstance() {
	static CommandBufferManager m_Instance;
	return m_Instance;
}

void CommandBufferManager::Init(DX12Context* context, UINT graphicsCount, UINT computeCount, UINT copyCount) {
	m_Context = context;
	ID3D12Device* device = context->Device.Get();

	m_CommandBuffers[GRAPHICS_TYPE].Init(device, GRAPHICS_TYPE, graphicsCount);
	m_CommandBuffers[COMPUTE_TYPE].Init(device, COMPUTE_TYPE, computeCount);
	m_CommandBuffers[COPY_TYPE].Init(device, COPY_TYPE, copyCount);

	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[GRAPHICS_TYPE])), L"Error creating fence");
	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[COMPUTE_TYPE])), L"Error creating fence");
	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[COPY_TYPE])), L"Error creating fence");
}

ID3D12GraphicsCommandList* CommandBufferManager::GetNextCommandList(COMMAND_BUFFER_TYPE type) {
	return m_CommandBuffers[type].GetNextCmdList();
}

void CommandBufferManager::ResetAllCommandBuffers() {
	for (auto& buffer : m_CommandBuffers) {
		buffer.ResetBuffer(m_Context->FrameIndex);
	}
}

void CommandBufferManager::ExecuteCommandBuffers(COMMAND_BUFFER_TYPE type) {
	ID3D12CommandQueue* queue;
	switch (type) {
	case GRAPHICS_TYPE:
		queue = m_Context->CommandQueue.Get();
		break;
	case COMPUTE_TYPE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case COPY_TYPE:
		queue = m_Context->CopyQueue.Get();
		break;
	}

	m_CommandBuffers[type].Execute(queue);
}

void CommandBufferManager::SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE sender, COMMAND_BUFFER_TYPE reciever){
	//if (sender == reciever) {
	//	printf("sender and receiver is the same\n");
	//}
	ID3D12CommandQueue* queue;
	switch (sender) {
	case GRAPHICS_TYPE:
		queue = m_Context->CommandQueue.Get();
		break;
	case COMPUTE_TYPE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case COPY_TYPE:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	queue->Signal(m_Fences[reciever].Get(), signal);
}

void CommandBufferManager::WaitOnFenceSignal(UINT64 signal, COMMAND_BUFFER_TYPE waiter) {
	ID3D12CommandQueue* queue;
	switch (waiter) {
	case GRAPHICS_TYPE:
		queue = m_Context->CommandQueue.Get();
		break;
	case COMPUTE_TYPE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case COPY_TYPE:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	queue->Wait(m_Fences[waiter].Get(), signal);
}