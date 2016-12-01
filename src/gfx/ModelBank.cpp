#include "ModelBank.h"
#include "MaterialBank.h"

ModelBank::ModelBank() {
	m_Numerator	= 0;
}

ModelBank::~ModelBank() {
	
}

ModelBank& ModelBank::GetInstance() {
	static ModelBank m_Bank;
	return m_Bank;
}

void ModelBank::Init() {
}

ModelHandle ModelBank::LoadModel(const char* filename) {
	//check if we already has a model with same filename
	for (auto& it : m_Models) {
		if (it.second.Name == filename)
			return it.first;
	}
	Model model;
	const aiScene* scene = m_Importer.ReadFile( filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_MakeLeftHanded | aiProcess_FlipUVs );
	if (scene && scene->HasMeshes()) {
		model.VertexHandle = m_VertexPositions.size();
		LoadMeshes(model, scene);
		model.MaterialHandle = g_MaterialBank.GetMaterialCount();
		model.Name = std::string(filename);
		g_MaterialBank.LoadMaterials(model, filename, scene);
	} else if (!scene) {
		printf("error loading model: %s\n ASSIMP Error: %s \n", filename, m_Importer.GetErrorString());
		return -1;
	}
	printf("Finished loading Model %s\n", filename);
	ModelHandle handle = ++m_Numerator;
	m_Models[handle] = model;
	return handle;
}

void ModelBank::LoadMeshes(Model& model, const aiScene* scene) {
	int size = 0;
	int indexCount = 0;
	int vertexCount = 0;
	Mesh modelMesh;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[i];
		//modelMesh.VertexBufferOffset = size;
		std::vector<unsigned int> indices;
		unsigned int numVertices = 0;
		unsigned int numIndices = 0;
		modelMesh.Max = glm::vec3(0);
		modelMesh.Min = glm::vec3(0);
		//foreach vertice
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
			glm::vec3 pos, normal, tangent;
			glm::vec2 texcoord;
			pos.x = mesh->mVertices[v].x;
			pos.y = mesh->mVertices[v].y;
			pos.z = mesh->mVertices[v].z;
			modelMesh.Max = glm::max(pos, modelMesh.Max);
			modelMesh.Min = glm::min(pos, modelMesh.Min);

			if (mesh->HasNormals()) {
				normal.x = mesh->mNormals[v].x;
				normal.y = mesh->mNormals[v].y;
				normal.z = mesh->mNormals[v].z;
			}
			if (mesh->HasTangentsAndBitangents()) {
				tangent.x = mesh->mTangents[v].x;
				tangent.y = mesh->mTangents[v].y;
				tangent.z = mesh->mTangents[v].z;
			}
			if (mesh->mTextureCoords[0] != NULL) {
				texcoord.x = mesh->mTextureCoords[0][v].x;
				texcoord.y = mesh->mTextureCoords[0][v].y;
			}
			numVertices++;
			m_VertexPositions.push_back(pos);
			m_VertexNormals.push_back(normal);
			m_VertexTangents.push_back(tangent);
			m_VertexTexCoords.push_back(texcoord);
		}//end foreach vertice
		//Indices
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			//index = (Num vertices from the already loaded models) + (Size of all the already loaded meshes + mesh->faceindices)
			indices.push_back(model.VertexHandle + size + mesh->mFaces[f].mIndices[0]);
			indices.push_back(model.VertexHandle + size + mesh->mFaces[f].mIndices[1]);
			indices.push_back(model.VertexHandle + size + mesh->mFaces[f].mIndices[2]);
			numIndices += 3;
		}
		m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());
		modelMesh.MaterialOffset = mesh->mMaterialIndex;
		modelMesh.IndexBufferOffset = indexCount;
		size += numVertices;
		indexCount += numIndices;
		modelMesh.VertexCount = numVertices;
		modelMesh.IndexCount = numIndices;

		model.Max = glm::max(modelMesh.Max, model.Max);
		model.Min = glm::min(modelMesh.Min, model.Min);

		glm::vec3 size = modelMesh.Max - modelMesh.Min;

		modelMesh.Offset = modelMesh.Min + (size * 0.5f);

		modelMesh.Max = size * 0.5f;
		modelMesh.Min = -size * 0.5f;
	
		modelMesh.Radius = glm::max(modelMesh.Max.x, glm::max(modelMesh.Max.y, modelMesh.Max.z));
		modelMesh.Radius = glm::max(modelMesh.Radius, glm::abs(glm::min(modelMesh.Min.x, glm::min(modelMesh.Min.y, modelMesh.Min.z))));

		model.Meshes.push_back(modelMesh);
	}//end foreach mesh

	model.NumVertices = size;
	model.NumIndices = indexCount;
}



ModelHandle ModelBank::AddModel( Model& TheModel ) {
	ModelHandle id = ++m_Numerator;
	m_Models[id] = TheModel;
	return id;
}

ModelHandle ModelBank::CreateCustomModel( std::vector<Vertex>* vertices, std::vector<UINT>* indices) {
	ModelHandle id = ++m_Numerator;
	Model model;
	Mesh mesh;
	mesh.IndexCount = (unsigned)indices->size();
	mesh.VertexCount = (unsigned)vertices->size();
	mesh.MaterialOffset = 0;
	mesh.IndexBufferOffset = 0;

	model.VertexHandle = m_VertexPositions.size();
	//copy and offset indices
	for (unsigned int i = 0; i < indices->size(); ++i) {
		m_Indices.push_back(model.VertexHandle + indices->at(i));
	}
	//copy vertices and calc aabb
	for (int i = 0; i < vertices->size(); ++i) {
		m_VertexPositions.push_back(vertices->at(i).Position);
		m_VertexNormals.push_back(vertices->at(i).Normal);
		m_VertexTangents.push_back(vertices->at(i).Tangent);
		m_VertexTexCoords.push_back(vertices->at(i).TexCoord);

		mesh.Max = glm::max(vertices->at(i).Position, mesh.Max);
		mesh.Min = glm::min(vertices->at(i).Position, mesh.Min);
	}

	mesh.Radius = glm::max(mesh.Max.x, glm::max(mesh.Max.y, mesh.Max.z));
	mesh.Radius = glm::max(mesh.Radius, glm::abs(glm::min(mesh.Min.x, glm::min(mesh.Min.y, mesh.Min.z))));
	model.Radius = mesh.Radius;

	model.Meshes.push_back(mesh);
	model.MaterialHandle = 0; //make sure there is a default material loaded
	model.IndexHandle = 0;
	model.NumIndices = (unsigned)indices->size();
	model.NumVertices = (unsigned)vertices->size();

	m_Models[id] = model;
	return id;
}

void ModelBank::DeleteModel( ) {
	//TODOHJ: Delete model data ,vertices and indices.
	//then update all the other models after it in the memory.
	// tbh its much easier and less cumbersome to just delete all models and load them in again.
}

Model& ModelBank::FetchModel(ModelHandle handle) {
	return m_Models[handle];
}

void ModelBank::Clear() {
	m_Models.clear();
	m_Indices.clear();
	m_Numerator = 0;
	m_Indices.clear();
	m_IndexBufferResource.Reset();
	m_VertexBufferResource.Reset();
}

void ModelBank::BuildBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
	// Vertex buffers
	size_t vertexCount = m_VertexPositions.size();
	size_t vboSize = (vertexCount) * (sizeof(glm::vec3) * 3 + sizeof(glm::vec2));

	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vboSize);
	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_VertexBufferResource)), L"Error creating Vertex buffer resource");

	CD3DX12_RESOURCE_DESC vertexBufferUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(vboSize);
	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&vertexBufferUploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBufferUpload)), L"Error creating Vertex buffer upload resource");
	
#ifdef _DEBUG
	m_VertexBufferResource->SetName(L"Vertex buffer");
	m_VertexBufferUpload->SetName(L"Vertex buffer upload heap");
#endif

	D3D12_GPU_VIRTUAL_ADDRESS posHandle,normalHandle, tangentHandle, texHandle;
	void* pData;
	m_VertexBufferUpload->Map(0, nullptr, &pData);
	size_t offset = 0;
	size_t size = m_VertexPositions.size() * sizeof(glm::vec3);
	//positions
	offset += size;
	posHandle = m_VertexBufferResource->GetGPUVirtualAddress();
	memcpy(pData, m_VertexPositions.data(), size);
	//normals
	normalHandle = m_VertexBufferResource->GetGPUVirtualAddress() + offset;
	pData = (void*)((UINT8*)pData + size);
	offset += size;
	memcpy(pData, m_VertexNormals.data(), size);
	//tangents
	tangentHandle = m_VertexBufferResource->GetGPUVirtualAddress() + offset;
	pData = (void*)((UINT8*)pData + size);
	offset += size;
	memcpy(pData, m_VertexTangents.data(), size);
	//texcoords
	texHandle = m_VertexBufferResource->GetGPUVirtualAddress() + offset;
	pData = (void*)((UINT8*)pData + size);
	size = m_VertexTexCoords.size() * sizeof(glm::vec2);
	offset += size;
	memcpy(pData, m_VertexTexCoords.data(), size);

	////create views for each mesh
	//int meshOffset = 0;
	//int vec3Size = sizeof(glm::vec3);
	//int vec2Size = sizeof(glm::vec2);
	//for (auto& model : m_Models) {
	//	for (auto& mesh : model.second.Meshes) {
	//		//pos
	//		mesh.VBOView.PosView.BufferLocation = posHandle + (meshOffset * vec3Size);
	//		mesh.VBOView.PosView.SizeInBytes = mesh.VertexCount * vec3Size;
	//		mesh.VBOView.PosView.StrideInBytes = vec3Size;
	//		//normal
	//		mesh.VBOView.NormalView.BufferLocation = normalHandle + (meshOffset * vec3Size);
	//		mesh.VBOView.NormalView.SizeInBytes = mesh.VertexCount * vec3Size;
	//		mesh.VBOView.NormalView.StrideInBytes = vec3Size;
	//		//tangent
	//		mesh.VBOView.TangentView.BufferLocation = tangentHandle + (meshOffset * vec3Size);
	//		mesh.VBOView.TangentView.SizeInBytes = mesh.VertexCount * vec3Size;
	//		mesh.VBOView.TangentView.StrideInBytes = vec3Size;
	//		//texcoord
	//		mesh.VBOView.TexView.BufferLocation = texHandle + (meshOffset * vec2Size);
	//		mesh.VBOView.TexView.SizeInBytes = mesh.VertexCount * vec2Size;
	//		mesh.VBOView.TexView.StrideInBytes = vec2Size;
	//		meshOffset += mesh.VertexCount;
	//	}
	//}
	//create vertex buffer view
	//position
	m_VertexBufferView[0].BufferLocation = posHandle;
	m_VertexBufferView[0].SizeInBytes = sizeof(glm::vec3) * m_VertexPositions.size();
	m_VertexBufferView[0].StrideInBytes = sizeof(glm::vec3);
	//normal
	m_VertexBufferView[1].BufferLocation = normalHandle;
	m_VertexBufferView[1].SizeInBytes = sizeof(glm::vec3) * m_VertexNormals.size();
	m_VertexBufferView[1].StrideInBytes = sizeof(glm::vec3);
	//tangent
	m_VertexBufferView[2].BufferLocation = tangentHandle;
	m_VertexBufferView[2].SizeInBytes = sizeof(glm::vec3) * m_VertexTangents.size();
	m_VertexBufferView[2].StrideInBytes = sizeof(glm::vec3);
	//uv
	m_VertexBufferView[3].BufferLocation = texHandle;
	m_VertexBufferView[3].SizeInBytes = sizeof(glm::vec2) * m_VertexTexCoords.size();
	m_VertexBufferView[3].StrideInBytes = sizeof(glm::vec2);


	cmdList->CopyResource(m_VertexBufferResource.Get() , m_VertexBufferUpload.Get());
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBufferResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdList->ResourceBarrier(1, &barrier);
	//Index Buffer
	int staticId = 0;
	for (std::map<ModelHandle, Model>::iterator it = m_Models.begin(); it != m_Models.end(); ++it) {
		it->second.IndexHandle = staticId;
		staticId += it->second.NumIndices;
	}

	size_t indexBufferSize = m_Indices.size() * sizeof(UINT);
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_IndexBufferResource)), L"Error creating index buffer resource");
	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexbufferUpload)), L"Error creating index buffer upload resource");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = &m_Indices[0];
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(cmdList, m_IndexBufferResource.Get(), m_IndexbufferUpload.Get(), 0, 0, 1, &indexData);
#ifdef _DEBUG
	m_IndexbufferUpload->SetName(L"Index buffer upload heap");
	m_IndexBufferResource->SetName(L"Index buffer");
#endif

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBufferResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdList->ResourceBarrier(1, &barrier);

	m_IndexBufferView.BufferLocation = m_IndexBufferResource->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = (unsigned)indexBufferSize;
}

void ModelBank::ApplyBuffers(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetIndexBuffer(&m_IndexBufferView);
	cmdList->IASetVertexBuffers(0, 4, m_VertexBufferView);
}

void ModelBank::ApplyVertexBuffers(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetVertexBuffers(0, 4, m_VertexBufferView);
}

void ModelBank::ApplyIndexBuffers(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetIndexBuffer(&m_IndexBufferView);
}

void ModelBank::FreeUploadHeaps(){
	m_IndexbufferUpload.Reset();
	m_VertexBufferUpload.Reset();
	m_VertexPositions.clear();
	m_VertexNormals.clear();
	m_VertexTangents.clear();
	m_VertexTexCoords.clear();
}

void ModelBank::UpdateModel(ModelHandle& handle, Model& model) {
	m_Models[handle] = model;
}
