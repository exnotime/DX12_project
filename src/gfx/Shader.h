#pragma once
#include "Common.h"
#define SHADER_TYPE_COUNT 6
enum SHADER_TYPES {
	VERTEX_SHADER_BIT	 = 1 << 0,
	PIXEL_SHADER_BIT	 = 1 << 1,
	GEOMETRY_SHADER_BIT	 = 1 << 2,
	HULL_SHADER_BIT		 = 1 << 3,
	DOMAIN_SHADER_BIT	 = 1 << 4,
	COMPUTE_SHADER_BIT	 = 1 << 5
};

class Shader {
public:
	Shader();
	~Shader();
	void LoadFromFile(const std::wstring& filename, UINT shaderTypes);
	D3D12_SHADER_BYTECODE GetByteCode(UINT shaderTypes);
private:
	std::vector<ID3DBlob*> m_ShaderBlobs;
};