#include "CommandBuffer.h"

CommandBuffer::CommandBuffer() {

}

CommandBuffer::~CommandBuffer() {
}

void CommandBuffer::Init(ID3D12Device* device, COMMAND_BUFFER_TYPE type) {
	D3D12_COMMAND_LIST_TYPE listType;
	switch (type){
	case CMD_BUFFER_TYPE_GRAPHICS:
		listType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case CMD_BUFFER_TYPE_COMPUTE:
		listType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	case CMD_BUFFER_TYPE_COPY:
		listType = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	}
	device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[0]));
	device->CreateCommandAllocator(listType, IID_PPV_ARGS(&m_CommandAllocators[1]));
	device->CreateCommandList(0, listType, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_CmdList));
	m_Type = type;
	m_Open = true;
}

void CommandBuffer::ResetBuffer(int frameIndex) {
	if (m_Active) {
		if (m_Open) {
			Close();
		}
		m_CommandAllocators[frameIndex]->Reset();
		m_CmdList->Reset(m_CommandAllocators[frameIndex].Get(), nullptr);
		m_Open = true;
		m_Active = false;
	}
}

void CommandBuffer::ResetCommandList(int frameIndex) {
	m_CmdList->Reset(m_CommandAllocators[frameIndex].Get(), nullptr);
	m_Open = true;
}

void CommandBuffer::Close() {
	m_CmdList->Close();
	m_Open = false;
}