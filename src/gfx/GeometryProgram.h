#pragma once
#include "DX12Common.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderQueue.h"
#include "FilterContext.h"
#include <atomic>
namespace GeometryProgram {
	struct GeometryProgramState {
		Shader Shader;
		ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12PipelineState> PipelineState;
		Texture DiffSkyTex;
		Texture SpecSkyTex;
		Texture IBLTex;
		ComPtr<ID3D12DescriptorHeap> SkyDescHeap;
		ComPtr<ID3D12DescriptorHeap> RenderDescHeap;
		UINT DescHeapIncSize;
		ComPtr<ID3D12CommandSignature>	CommandSignature;
		std::atomic_uint_fast32_t DescCounter;
	};

	enum ROOT_PARAMS {
		PER_FRAME_CONST_BUFFER,
		SHADER_INPUT_STRUCT_BUFFER,
		DRAW_INDEX_CONSTANT,
		ENVIROMENT_DESC_TABLE,
		MATERIAL_DESC_TABLE,
		ROOT_PARAMS_SIZE
	};

	const int MAX_RENDER_OBJECTS = 10000;
	const int MATERIAL_SIZE = 4;
	const int ENVIRONMENT_MATERIAL_SIZE = 3;
};

void InitGeometryState(GeometryProgram::GeometryProgramState* state, DX12Context* context, ID3D12GraphicsCommandList* cmdList);
void RenderGeometry(ID3D12GraphicsCommandList*cmdList,
	GeometryProgram::GeometryProgramState* state, RenderQueue* queue, FilterContext* filter);