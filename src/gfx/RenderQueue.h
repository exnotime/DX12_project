#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Common.h"
#include "Camera.h"
#include "BufferManager.h"
typedef int ModelHandle;
typedef unsigned short TerrainHandle;

struct ModelObject {
	ModelHandle Model;
	unsigned int InstanceCount;
};

struct TransparentModelObject {
	ModelHandle Model;
	unsigned int InstanceCount;
	float Transparency;
};

struct ShaderInput {
	glm::mat4 World;
	glm::vec4 Color;
};

struct Viewport {
	float x;
	float y;
	float width;
	float height;
};

struct View {
	CameraData Camera;
	Viewport Viewport;
};

struct IndirectDrawCall {
	UINT DrawIndex;
	UINT MaterialOffset;
	D3D12_DRAW_INDEXED_ARGUMENTS DrawArgs;
};

class RenderQueue {
  public:
	RenderQueue();
	~RenderQueue();
	void Init(DX12Context* context);
	void Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs);
	void Enqueue(ModelHandle model, const ShaderInput& input);

	void EnqueueOccluder(ModelHandle occluderModel);

	void Clear();
	void UpdateBuffer();

	void AddView(const View& v) {
		m_Views.push_back(v);
	}

	const std::vector<View>& GetViews() const {
		return m_Views;
	}

	UINT GetDrawCount() const {
		return m_DrawCalls.size();
	}

	UINT GetOccluderCount() const {
		return m_OccluderDrawCalls.size();
	}

  private:
	std::vector<View>					m_Views;
	std::vector<ShaderInput>			m_ShaderInputBuffer;
	
	DX12Context*						m_Context;

	std::vector<IndirectDrawCall>		m_DrawCalls;
	std::vector<IndirectDrawCall>		m_OccluderDrawCalls;
	unsigned							m_InstanceCounter;
	const int MAX_SHADER_INPUT_COUNT = 10000;
};