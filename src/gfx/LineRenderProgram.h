#pragma once
#include "DX12Common.h"
#include "RenderQueue.h"
#include "Shader.h"
class LineRenderProgram {
public:
	LineRenderProgram();
	~LineRenderProgram();
	void Init(ID3D12Device* device);
	void Render(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue);
private:
	Shader m_Shader;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12Resource> m_BufferResource;
};