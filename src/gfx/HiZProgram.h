#pragma once
#include "Common.h"
#include "Shader.h"

class HiZProgram {
public:
	HiZProgram();
	~HiZProgram();
	void Init(DX12Context* context, glm::vec2 screenSize);
	void Disbatch(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* srcTex);
	ID3D12Resource* GetResource() {
		return m_HiZResource.Get();
	}

	UINT GetMipCount() {
		return m_MipCount;
	}
private:
	Shader m_Shader;
	ComPtr<ID3D12RootSignature>		m_RootSignature;
	ComPtr<ID3D12PipelineState>		m_PipelineState;
	ComPtr<ID3D12DescriptorHeap>	m_DescHeap;
	ComPtr<ID3D12Resource>			m_HiZResource;
	UINT							m_MipCount;
	UINT							m_DescIncSize;
	glm::vec2						m_ScreenSize;
};