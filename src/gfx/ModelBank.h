#pragma once
#include "Model.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "DX12Common.h"
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef int ModelHandle;
#define g_ModelBank ModelBank::GetInstance()
class ModelBank {
  public:
	~ModelBank();
	static ModelBank& GetInstance();
	void Init();
	Model& FetchModel(ModelHandle handle);
	ModelHandle LoadModel(const char* filename);
	ModelHandle AddModel(Model& TheModel);
	ModelHandle CreateCustomModel(std::vector<Vertex>* vertices, std::vector<UINT>* indices);
	void UpdateModel(ModelHandle& handle, Model& model);
	void BuildBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void DeleteModel();
	void ApplyBuffers(ID3D12GraphicsCommandList* cmdList);
	void ApplyVertexBuffers(ID3D12GraphicsCommandList* cmdList);
	void ApplyIndexBuffers(ID3D12GraphicsCommandList* cmdList);
	void Clear();
	void FreeUploadHeaps();

	UINT GetVertexCount() {
		return m_VertexPositions.size();
	}
	ID3D12Resource* GetVertexBufferResource() {
		return m_VertexBufferResource.Get();
	}
	UINT GetIndexCount() {
		return m_Indices.size();
	}
	ID3D12Resource* GetIndexBufferResource() {
		return m_IndexBufferResource.Get();
	}

  private:
	ModelBank();
	void LoadMeshes(Model& model, const aiScene* scene);

	Assimp::Importer				m_Importer;
	ModelHandle						m_Numerator;
	std::map<ModelHandle, Model>	m_Models;
	//vertices
	std::vector<glm::vec3>			m_VertexPositions;
	std::vector<glm::vec3>			m_VertexNormals;
	std::vector<glm::vec3>			m_VertexTangents;
	std::vector<glm::vec2>			m_VertexTexCoords;

	//indices
	std::vector<UINT>				m_Indices;

	D3D12_INDEX_BUFFER_VIEW			m_IndexBufferView;
	ComPtr<ID3D12Resource>			m_IndexBufferResource;
	ComPtr<ID3D12Resource>			m_IndexbufferUpload;

	D3D12_VERTEX_BUFFER_VIEW		m_VertexBufferView[4];
	ComPtr<ID3D12Resource>			m_VertexBufferResource;
	ComPtr<ID3D12Resource>			m_VertexBufferUpload;

};
