#pragma once

#include "Shader.h"
#include "Texture.h"
#include "RenderQueue.h"
namespace DepthOnlyProgram {
	struct DepthOnlyState {
		Shader Shader;
		ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12PipelineState> PipelineState;
		ComPtr<ID3D12CommandSignature>	CommandSignature;
	};

	enum ROOT_PARAMS {
		PER_FRAME_CB,
		SHADER_INPUT_SB,
		DRAW_INDEX_C,
		ROOT_PARAMS_SIZE
	};
};

void InitDepthOnlyState(DepthOnlyProgram::DepthOnlyState* state, DX12Context* context);
void DepthOnlyRender(ID3D12GraphicsCommandList*cmdList,
	DepthOnlyProgram::DepthOnlyState* state, RenderQueue* queue);