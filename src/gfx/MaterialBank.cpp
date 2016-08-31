#include "MaterialBank.h"
#include <fstream>
#include <assimp/scene.h>
#include "Model.h"
#include "BufferManager.h"

static std::string GetDirectoryFromFilePath(const std::string& filePath) {
	int lastSlashPos = static_cast<int>(filePath.rfind('/'));
	return filePath.substr(0, lastSlashPos);
}

MaterialBank::MaterialBank() {
}

MaterialBank::~MaterialBank() {
	//ClearMaterials();
}

MaterialBank& MaterialBank::GetInstance() {
	static MaterialBank m_Bank;
	return m_Bank;
}

void MaterialBank::Initialize(DX12Context* context){
	m_Context = context;
	m_DescSize = context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_MaterialCount = 0;

	m_DefaultAlbedo = LoadTexture("assets/textures/albedo.png");
	m_DefaultNormal = LoadTexture("assets/textures/normal.png");
	m_DefaultRoughness = LoadTexture("assets/textures/roughness.png");
	m_DefaultMetal = LoadTexture("assets/textures/metallic.png");

	m_DefaultMaterial = new Material();

	m_DefaultMaterial->Offset = m_MaterialCount;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = MAX_MATERIAL_COUNT * 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(m_Context->Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_MaterialHeap)), L"Error creating descriptor heap for material");

	//CreateDescHeap(m_DefaultMaterial);
	m_MaterialHandle = m_MaterialHeap->GetCPUDescriptorHandleForHeapStart();

	m_Textures[m_DefaultAlbedo]->CreateSRV(context, m_MaterialHandle);
	m_Textures[m_DefaultNormal]->CreateSRV(context, m_MaterialHandle.Offset(1, m_DescSize));
	m_Textures[m_DefaultRoughness]->CreateSRV(context, m_MaterialHandle.Offset(1, m_DescSize));
	m_Textures[m_DefaultMetal]->CreateSRV(context, m_MaterialHandle.Offset(1, m_DescSize));
	m_Materials.push_back(m_DefaultMaterial);
	m_MaterialCount++;
	m_Updated = true;
}

void MaterialBank::LoadMaterials(Model& model, std::string filename, const aiScene* scene) {
	if (scene->mNumMaterials == 0) {
		model.MaterialHandle = 0;
		return;
	}
	model.MaterialHandle = (unsigned int)m_Materials.size();
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		Material* modelMat = new Material();
		modelMat->Offset = m_MaterialCount;
		const aiMaterial* mat = scene->mMaterials[i];

#ifdef _DEBUG
		aiString name;
		mat->Get(AI_MATKEY_NAME, name);
		modelMat->Name = name.C_Str();
#endif
		//albedo
		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString path;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string fullpath = GetDirectoryFromFilePath(filename) + "/" + path.data;
				TextureHandle a = LoadTexture(fullpath.c_str());
				m_Textures[a]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
			}
		} else {
			m_Textures[m_DefaultAlbedo]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
		}
		//normal map
		if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
			aiString path;
			if (mat->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string fullpath = GetDirectoryFromFilePath(filename) + "/" + path.data;
				TextureHandle n = LoadTexture(fullpath.c_str());
				m_Textures[n]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
			}
		} else {
			m_Textures[m_DefaultNormal]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
		}
		//roughness map
		if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			aiString path;
			if (mat->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string fullpath = GetDirectoryFromFilePath(filename) + "/" + path.data;
				TextureHandle r = LoadTexture(fullpath.c_str());
				m_Textures[r]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
			}
		} else {
			m_Textures[m_DefaultRoughness]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
		}
		//Metal map
		if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) { //use ambient texture as metal map for now
			aiString path;
			if (mat->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string fullpath = GetDirectoryFromFilePath(filename) + "/" + path.data;
				TextureHandle m = LoadTexture(fullpath.c_str());
				m_Textures[m]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
			}
		} else {
			m_Textures[m_DefaultMetal]->CreateSRV(m_Context, m_MaterialHandle.Offset(1, m_DescSize));
		}
		m_Materials.push_back(modelMat);
		m_MaterialCount++;
	}
	m_Updated = true;
}

void MaterialBank::ClearMaterials() {
	for (int i = 0; i < m_Materials.size(); i++) {
		if (m_Materials[i]) delete m_Materials[i];
	}
	for (int i = 0; i < m_Textures.size(); i++) {
		if (m_Textures[i]) delete m_Textures[i];
	}
	m_Materials.clear();
	m_MatMap.clear();
	m_Updated = true;
}


Material* MaterialBank::GetMaterial(int matIndex) {
	if(matIndex == -1)
		return nullptr;
	return m_Materials[matIndex];
}

Material* MaterialBank::GetMaterial(const std::string& name) {
	std::map<std::string, Material*>::iterator it = m_MatMap.find(name);
	if(it != m_MatMap.end()) {
		return m_MatMap[name];
	} else
		return nullptr;
}

TextureHandle MaterialBank::LoadTexture(const char* filename) {
	Texture* tex = new Texture();
	tex->Init(filename, m_Context);
	m_Textures.push_back(tex);
	return m_Numerator++;

	m_Updated = true;
}

Texture* MaterialBank::GetTexture(TextureHandle handle) {
	return m_Textures[handle];
}

void MaterialBank::FreeResources() {
	for (auto& tex : m_Textures) {
		tex->FreeUploadHeap();
	}
}

void MaterialBank::CopyMaterialDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE dest) {
	m_Context->Device->CopyDescriptorsSimple(m_MaterialCount * 4, dest, m_MaterialHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}