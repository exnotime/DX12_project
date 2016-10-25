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

	

	m_CmdLists.resize(cmdListCount);
	m_CommandAllocators[0].resize(cmdListCount);
	m_CommandAllocators[1].resize(cmdListCount);
	for (int i = 0; i < cmdListCount; i++) {
		device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[0][i]));
		device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[1][i]));

		device->CreateCommandList(0, listType, m_CommandAllocators[0][i].Get(), nullptr, IID_PPV_ARGS(&m_CmdLists[i]));
		//m_CmdLists[i]->Close();
		//m_CommandAllocators[0][i]->Reset();
	}
}

void CommandBuffer::ResetBuffer(int frameIndex) {
	for (int i = 0; i < m_ClosedCmdLists.size(); ++i) {
		m_CommandAllocators[frameIndex][i]->Reset();
		m_ClosedCmdLists[i]->Reset(m_CommandAllocators[frameIndex][i].Get(), nullptr);
	}
	m_ClosedCmdLists.clear();
	m_Numerator = 0;
}

ID3D12GraphicsCommandList* CommandBuffer::GetNextCmdList() {
	ID3D12GraphicsCommandList* list = m_CmdLists[m_Numerator].Get();
	//list->Reset(m_CommandAllocators[0][m_Numerator].Get(), nullptr);
	m_Numerator++;
	m_ClosedCmdLists.push_back(list);
	return list;
}

void CommandBuffer::Execute(ID3D12CommandQueue* queue) {
	queue->ExecuteCommandLists(m_ClosedCmdLists.size(), reinterpret_cast<ID3D12CommandList* const*>(m_ClosedCmdLists.data()));
}