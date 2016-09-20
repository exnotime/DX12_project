#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include <d3d12.h>

struct Mesh {
	unsigned int MaterialOffset;
	unsigned int IndexBufferOffset;
	unsigned int IndexCount;
	unsigned int VertexCount;
};


struct BoneView {
	D3D12_VERTEX_BUFFER_VIEW BoneIndexView;
	D3D12_VERTEX_BUFFER_VIEW BoneWeightView;
};

struct Model {
	unsigned int BoneHandle; //place in BoneBuffer
	unsigned int IndexHandle; //place in IndexBuffer
	unsigned int VertexHandle; //place in VertexBuffer
	unsigned int MaterialHandle; //place in MaterialBuffer
	std::string Name;
	unsigned int NumVertices;
	unsigned int NumIndices;
	unsigned int NumBones;
	glm::vec3 Max;
	glm::vec3 Min;
	float Radius;
	std::vector<Mesh> Meshes;
};

struct Bone {
	std::string Name;
	unsigned int Parent;
	std::vector<unsigned int> Children;
	glm::mat4 Offset;
	glm::mat4 FinalTransform;
};

struct Skelleton {
	glm::mat4 GlobalInvTransform;
	std::vector<Bone> Bones;
	std::map<std::string, int> BoneMapping;
};

struct Pose {
	float Time;
	std::vector<glm::mat4> Joints;
};