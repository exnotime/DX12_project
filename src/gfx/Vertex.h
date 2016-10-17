#pragma once
#include <glm/glm.hpp>
#include <d3d12.h>
#include <dxgi1_5.h>
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoord;
};

const int VertexLayoutSize = 4; 
const D3D12_INPUT_ELEMENT_DESC VertexLayout[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

const int DepthOnlyVertexSize = 1;
const D3D12_INPUT_ELEMENT_DESC DepthOnlyVertexLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

struct AnimatedVertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoord;
	glm::vec4 Weights;
	glm::uvec4 Bones;
};

const int AnimatedVertexLayoutSize = 6;
const D3D12_INPUT_ELEMENT_DESC AnimatedVertexLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};