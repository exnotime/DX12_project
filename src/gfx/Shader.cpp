#include "Shader.h"
#include <d3dcompiler.h>
Shader::Shader() {
	m_ShaderBlobs.resize(SHADER_TYPE_COUNT);
}

Shader::~Shader() {

}

void Shader::LoadFromFile(const std::wstring& filename, UINT shaderTypes) {

	m_ShaderTypes = shaderTypes;
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#endif

	ID3DBlob* errorBlob = nullptr;
	UINT index;
	if (shaderTypes & VERTEX_SHADER_BIT) {
		index = log2((float)VERTEX_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling vertex shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & PIXEL_SHADER_BIT) {
		index = log2((float)PIXEL_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling pixel shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & GEOMETRY_SHADER_BIT) {
		index = log2((float)GEOMETRY_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "GSMain", "gs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling geometry shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & HULL_SHADER_BIT) {
		index = log2((float)HULL_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "HSMain", "hs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling hull shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & DOMAIN_SHADER_BIT) {
		index = log2((float)DOMAIN_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "DSMain", "ds_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling domain shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & COMPUTE_SHADER_BIT) {
		index = log2((float)COMPUTE_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "CSMain", "cs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling compute shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	wprintf(L"Compiled shader: %s \n", filename.c_str());
}

D3D12_SHADER_BYTECODE Shader::GetByteCode(UINT shaderType) const {
	UINT index = log2(shaderType);
	D3D12_SHADER_BYTECODE byteCode;
	byteCode.pShaderBytecode = reinterpret_cast<UINT8*>(m_ShaderBlobs.at(index)->GetBufferPointer());
	byteCode.BytecodeLength = m_ShaderBlobs.at(index)->GetBufferSize();
	return byteCode;
}