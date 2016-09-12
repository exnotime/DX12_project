#pragma once
#define g_MaterialBank MaterialBank::GetInstance()
#include <map>
#include <vector>
#include "Texture.h"
#include "Material.h"

struct aiScene;
typedef unsigned int TextureHandle;
struct Model;
#define MAX_MATERIAL_COUNT 1000
#define MATERIAL_SIZE 4

class MaterialBank {
  public:
	~MaterialBank();
 	static MaterialBank& GetInstance();
	void Initialize(DX12Context* context);
	void LoadMaterials(Model& model, std::string filename, const aiScene* scene);
	void ClearMaterials();
	Material* GetMaterial(int matIndex);
	Material* GetMaterial(const std::string& name);
	TextureHandle LoadTexture(const char* filename);
	Texture* GetTexture(TextureHandle handle);
	int GetMaterialCount() {
		return (int)m_Materials.size();
	}
	void FreeResources();
	void CopyMaterialDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE dest);

  private:
	MaterialBank();
	int												m_Numerator = 0;
	std::vector<Material*>							m_Materials;
	std::map<std::string, Material*>				m_MatMap;
	std::vector<Texture*>							m_Textures;
	TextureHandle									m_DefaultAlbedo;
	TextureHandle									m_DefaultNormal;
	TextureHandle									m_DefaultRoughness;
	TextureHandle									m_DefaultMetal;
	Material*										m_DefaultMaterial;
	bool											m_Updated = false;

	DX12Context*									m_Context;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					m_MaterialHandle;
	ComPtr<ID3D12DescriptorHeap>					m_MaterialHeap;
	UINT											m_MaterialCount;
	UINT											m_DescSize;
};
