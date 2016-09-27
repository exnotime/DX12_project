#pragma once

#include "Shader.h"
#include "Texture.h"
#include "RenderQueue.h"
class DepthOnlyProgram {
public:
	DepthOnlyProgram();
	~DepthOnlyProgram();
	void Init(DX12Context* context, glm::vec2 screenSize);
	void Render(ID3D12GraphicsCommandList*cmdList, RenderQueue* queue);

	ID3D12Resource* GetDepthTexture() {
		return m_DepthTexture.Get();
	}

private:
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12CommandSignature>	m_CommandSignature;
	ComPtr<ID3D12DescriptorHeap> m_MaterialHeap;
	ComPtr<ID3D12Resource> m_DepthTexture;
	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	glm::vec2 m_ScreenSize;
	D3D12_RECT m_ScissorRect;
	D3D12_VIEWPORT m_Viewport;

	enum ROOT_PARAMS {
		PER_FRAME_CB,
		SHADER_INPUT_SB,
		DRAW_INDEX_C,
		MATERIAL_DT,
		ROOT_PARAMS_SIZE
	};
};