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

	m_GraphicsBuffers.resize(graphicsCount);
	for (int i = 0; i < graphicsCount; i++) {
		m_GraphicsBuffers[i].Init(device, CMD_BUFFER_TYPE_GRAPHICS);
	}
	m_ComputeBuffers.resize(computeCount);
	for (int i = 0; i < computeCount; i++) {
		m_ComputeBuffers[i].Init(device, CMD_BUFFER_TYPE_COMPUTE);
	}
	m_CopyBuffers.resize(copyCount);
	for (int i = 0; i < copyCount; i++) {
		m_CopyBuffers[i].Init(device, CMD_BUFFER_TYPE_COPY);
	}

	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[CMD_BUFFER_TYPE_GRAPHICS])), L"Error creating fence");
	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[CMD_BUFFER_TYPE_COMPUTE])), L"Error creating fence");
	HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[CMD_BUFFER_TYPE_COPY])), L"Error creating fence");

	m_BufferCounters[CMD_BUFFER_TYPE_GRAPHICS] = 0;
	m_BufferCounters[CMD_BUFFER_TYPE_COMPUTE] = 0;
	m_BufferCounters[CMD_BUFFER_TYPE_COPY] = 0;

}

CommandBuffer* CommandBufferManager::GetNextCommandBuffer(COMMAND_BUFFER_TYPE type) {
	CommandBuffer* cmdBuffer = nullptr;
	switch (type) {
	case CMD_BUFFER_TYPE_GRAPHICS:
		cmdBuffer = &m_GraphicsBuffers[m_BufferCounters[type]++];
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		cmdBuffer = &m_ComputeBuffers[m_BufferCounters[type]++];
		break;
	case CMD_BUFFER_TYPE_COPY:
		cmdBuffer = &m_CopyBuffers[m_BufferCounters[type]++];
		break;
	}
	if (cmdBuffer) cmdBuffer->Activate();
	return cmdBuffer;
}

void CommandBufferManager::ResetAllCommandBuffers() {

	for (auto& buffer : m_GraphicsBuffers) {
		buffer.ResetBuffer(m_Context->FrameIndex);
	}
	for (auto& buffer : m_ComputeBuffers) {
		buffer.ResetBuffer(m_Context->FrameIndex);
	}
	for (auto& buffer : m_CopyBuffers) {
		buffer.ResetBuffer(m_Context->FrameIndex);
	}
	m_BufferCounters[CMD_BUFFER_TYPE_GRAPHICS] = 0;
	m_BufferCounters[CMD_BUFFER_TYPE_COMPUTE] = 0;
	m_BufferCounters[CMD_BUFFER_TYPE_COPY] = 0;
}

void CommandBufferManager::ExecuteCommandBuffers(const std::vector<CommandBuffer*>& buffers, COMMAND_BUFFER_TYPE type) {
	ID3D12CommandQueue* queue;
	switch (type) {
	case CMD_BUFFER_TYPE_GRAPHICS:
		queue = m_Context->CommandQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COPY:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	std::vector<ID3D12CommandList*> cmdLists;
	//build list of cmdlists
	for (auto& buffer : buffers) {
		buffer->Close();
		cmdLists.push_back(buffer->CmdList());
		//should check if cmdbuffer type matches queue type
	}
	queue->ExecuteCommandLists(cmdLists.size(), reinterpret_cast<ID3D12CommandList* const*>(cmdLists.data()));
}

void CommandBufferManager::ExecuteCommandBuffer(CommandBuffer* buffer, COMMAND_BUFFER_TYPE type) {
	ID3D12CommandQueue* queue;
	switch (type) {
	case CMD_BUFFER_TYPE_GRAPHICS:
		queue = m_Context->CommandQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COPY:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	buffer->Close();
	ID3D12CommandList* cmdLists[] = { buffer->CmdList() };
	queue->ExecuteCommandLists(1, cmdLists);
}

void CommandBufferManager::SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE sender, COMMAND_BUFFER_TYPE reciever){
	//if (sender == reciever) {
	//	printf("sender and receiver is the same\n");
	//}
	ID3D12CommandQueue* queue;
	switch (sender) {
	case CMD_BUFFER_TYPE_GRAPHICS:
		queue = m_Context->CommandQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COPY:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	queue->Signal(m_Fences[reciever].Get(), signal);
}

void CommandBufferManager::SignalFence(UINT64 signal, COMMAND_BUFFER_TYPE receiver) {
	m_Fences[receiver]->Signal(signal);
}

void CommandBufferManager::WaitOnFenceSignal(UINT64 signal, COMMAND_BUFFER_TYPE waiter) {
	ID3D12CommandQueue* queue;
	switch (waiter) {
	case CMD_BUFFER_TYPE_GRAPHICS:
		queue = m_Context->CommandQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		queue = m_Context->ComputeQueue.Get();
		break;
	case CMD_BUFFER_TYPE_COPY:
		queue = m_Context->CopyQueue.Get();
		break;
	}
	queue->Wait(m_Fences[waiter].Get(), signal);
}