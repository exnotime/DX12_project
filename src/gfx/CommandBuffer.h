#pragma once
#include "DX12Common.h"
enum COMMAND_BUFFER_TYPE {
	CMD_BUFFER_TYPE_GRAPHICS,
	CMD_BUFFER_TYPE_COMPUTE,
	CMD_BUFFER_TYPE_COPY,
	CMD_BUFFER_TYPE_COUNT
};

class CommandBuffer {
public:
	CommandBuffer();
	~CommandBuffer();
	void Init(ID3D12Device* device, COMMAND_BUFFER_TYPE type);
	void Activate() { m_Active = true; }
	void Close();
	void ResetBuffer(int frameIndex);
	void ResetCommandList(int frameIndex);
	ID3D12GraphicsCommandList* CmdList() { return m_CmdList.Get(); }

private:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocators[g_FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_CmdList;
	COMMAND_BUFFER_TYPE m_Type;
	bool m_Open;
	bool m_Active;
};