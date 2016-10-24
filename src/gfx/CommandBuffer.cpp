#include "CommandBuffer.h"

CommandBuffer::CommandBuffer() {

}

CommandBuffer::~CommandBuffer() {

}

void CommandBuffer::Init(ID3D12Device* device, COMMAND_BUFFER_TYPE type, UINT cmdListCount) {
	D3D12_COMMAND_LIST_TYPE listType;
	switch (type)
	{
	case GRAPHICS_TYPE:
		listType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case COMPUTE_TYPE:
		listType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	case COPY_TYPE:
		listType = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	}

	device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[0]));
	device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[1]));

	m_CmdLists.resize(cmdListCount);
	for (int i = 0; i < cmdListCount; i++) {
		device->CreateCommandList(0, listType, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_CmdLists[i]));
		m_CmdLists[i]->Close();
		m_CommandAllocators[0]->Reset();
	}
}

void CommandBuffer::ResetAllocator(int frameIndex) {
	m_CommandAllocators[frameIndex]->Reset();
}

void CommandBuffer::ResetCommandLists(int frameIndex) {
	for (auto& list : m_ClosedCmdLists) {
		list->Reset(m_CommandAllocators[frameIndex].Get(), nullptr);
	}
	m_ClosedCmdLists.clear();
	m_Numerator = 0;
}

ID3D12GraphicsCommandList* CommandBuffer::GetNextCmdList() {
	ID3D12GraphicsCommandList* list = m_CmdLists[m_Numerator].Get();
	m_Numerator++;
	m_ClosedCmdLists.push_back(list);
	return list;
}

void CommandBuffer::Execute(ID3D12CommandQueue* queue) {
	queue->ExecuteCommandLists(m_ClosedCmdLists.size(), reinterpret_cast<ID3D12CommandList* const*>(m_ClosedCmdLists.data()));
}