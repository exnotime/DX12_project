#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Common.h"
#include "Camera.h"
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
	D3D12_VERTEX_BUFFER_VIEW VBO[4];
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
	void Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs, float transparency);
	void Enqueue(ModelHandle model, const ShaderInput& input, float transparency);
	void Clear();
	void CreateBuffer();
	void UpdateBuffer();

	void AddView(const View& v) {
		m_Views.push_back(v);
	}
	const std::vector<ModelObject>& GetModelQueue() const {
		return m_ModelQueue;
	}
	const std::vector<TransparentModelObject>& GetTranparentModelQueue() const {
		return m_TransparentModelQueue;
	}
	const std::vector<View>& GetViews() const {
		return m_Views;
	}

	ID3D12Resource* GetArgumentBuffer() const {
		return m_ArgumentBuffer.Get();
	}

	UINT GetDrawCount() const {
		return m_IndirectCounter;
	}

  private:
	std::vector<ModelObject>			m_ModelQueue;
	std::vector<TransparentModelObject>	m_TransparentModelQueue;
	std::vector<View>					m_Views;
	std::vector<ShaderInput>			m_ShaderInputBuffer;
	std::vector<ShaderInput>			m_TransparentShaderInputBuffer;
	
	DX12Context*						m_Context;
	ComPtr<ID3D12Resource>				m_ArgumentBuffer;
	ComPtr<ID3D12Resource>				m_ArgumentUpload;
	IndirectDrawCall*					m_IndirectStart;
	UINT								m_IndirectCounter;
	UINT								m_InstanceCounter;

	const int MAX_SHADER_INPUT_COUNT = 100;
};