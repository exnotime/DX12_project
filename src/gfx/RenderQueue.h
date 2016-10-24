#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "DX12Common.h"
#include "Camera.h"
#include "BufferManager.h"
typedef int ModelHandle;
typedef unsigned short TerrainHandle;

struct ModelObject {
	ModelHandle Model;
	unsigned int InstanceCount;
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
	UINT Padding;
};

class RenderQueue {
  public:
	RenderQueue();
	~RenderQueue();
	void Init();
	void Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs);
	void Enqueue(ModelHandle model, const ShaderInput& input);

	void EnqueueOccluder(ModelHandle occluderModel);

	void Clear();
	void UpdateBuffer(ID3D12GraphicsCommandList* cmdList);

	void AddView(const View& v);

	const std::vector<View>& GetViews() const {
		return m_Views;
	}

	UINT GetDrawCount() const {
		return m_DrawCalls.size();
	}

	UINT GetOccluderCount() const {
		return m_OccluderDrawCalls.size();
	}
	std::vector<IndirectDrawCall>& GetDrawList() {
		return m_DrawCalls;
	}

  private:
	  bool AABBvsFrustum(const glm::vec4& max, const glm::vec4& min);
	  bool SpherevsFrustum(const glm::vec4& pos, const float radius);

	std::vector<View>					m_Views;
	glm::vec4							m_FrustumPlanes[6];
	glm::vec4							m_FrustumCorners[8];

	std::vector<ShaderInput>			m_ShaderInputBuffer;
	
	DX12Context*						m_Context;

	std::vector<IndirectDrawCall>		m_DrawCalls;
	std::vector<IndirectDrawCall>		m_OccluderDrawCalls;
	unsigned							m_InstanceCounter;
	const int MAX_SHADER_INPUT_COUNT = 10000;
};