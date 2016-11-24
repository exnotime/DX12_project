#include "Shader.h"
#include <d3dcompiler.h>
Shader::Shader() {
	m_ShaderBlobs.resize(SHADER_TYPE_COUNT);
}

Shader::~Shader() {

}

void Shader::LoadFromFile(const std::wstring& filename, UINT shaderTypes, ExtensionContext* extensions, std::vector<D3D_SHADER_MACRO>* macros) {

	m_ShaderTypes = shaderTypes;
#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG;
#else
	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#endif

	std::vector<D3D_SHADER_MACRO> shaderMacros;
	if (macros) {
		shaderMacros.insert(shaderMacros.begin(), macros->begin(), macros->end());
	}

	if (extensions) {
		//AMD
		if (extensions->Vendor == AMD_VENDOR_ID) {
			compileFlags &= ~D3DCOMPILE_SKIP_OPTIMIZATION;
			shaderMacros.push_back({"AMD_USE_SHADER_INTRINSICS", "1"});
			shaderMacros.push_back({ nullptr, nullptr });
		} //NVIDIA
		else if (extensions->Vendor == NVIDIA_VENDOR_ID) {
			compileFlags &= ~D3DCOMPILE_SKIP_OPTIMIZATION;
			shaderMacros.push_back({ "NVIDIA_USE_SHADER_INTRINSICS", "1" });
			shaderMacros.push_back({ nullptr, nullptr });
		}
	}

	D3D_SHADER_MACRO* macro = (shaderMacros.size() > 0 ? shaderMacros.data() : nullptr);

	ID3DBlob* errorBlob = nullptr;
	UINT index;
	if (shaderTypes & VERTEX_SHADER_BIT) {
		index = log2((float)VERTEX_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling vertex shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & PIXEL_SHADER_BIT) {
		index = log2((float)PIXEL_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling pixel shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & GEOMETRY_SHADER_BIT) {
		index = log2((float)GEOMETRY_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling geometry shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & HULL_SHADER_BIT) {
		index = log2((float)HULL_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "HSMain", "hs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling hull shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & DOMAIN_SHADER_BIT) {
		index = log2((float)DOMAIN_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DSMain", "ds_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
			printf("Error compiling domain shader\nErrorLog: %s\n", errorBlob->GetBufferPointer());
		}
	}
	if (shaderTypes & COMPUTE_SHADER_BIT) {
		index = log2((float)COMPUTE_SHADER_BIT);
		if (D3DCompileFromFile(filename.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_1", compileFlags, 0, &m_ShaderBlobs[index], &errorBlob) != S_OK) {
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