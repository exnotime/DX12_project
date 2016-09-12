#include "ModelBank.h"
#include "MaterialBank.h"
#include "Animation.h"
#include <functional>

ModelBank::ModelBank() {
	m_Numerator	= 0;
}

ModelBank::~ModelBank() {
	

}

ModelBank& ModelBank::GetInstance() {
	static ModelBank m_Bank;
	return m_Bank;
}

void ModelBank::Init(DX12Context* context) {
	m_Context = context;
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
		//model.VertexHandle = (int)m_Vertices.size();
		if (scene->HasAnimations()) {
			LoadRiggedMeshes(model, scene);
			Animation anim;
			Skelleton skel = LoadSkelleton(scene);
			anim.Load(scene->mAnimations[0], skel);
			anim.CalcPose(1.2f);
		} else {
			LoadMeshes(model, scene);
			model.Radius = glm::max(model.Max.x, glm::max(model.Max.y, model.Max.z));
		}
		model.MaterialHandle = g_MaterialBank.GetMaterialCount();
		model.Name = std::string(filename);
		g_MaterialBank.LoadMaterials(model, filename, scene);
	} else if (!scene) {
		printf("error loading model: %s \n", filename);
		return -1;
	}
	printf("Loaded Model %s\n", filename);
	ModelHandle handle = ++m_Numerator;
	m_Models[handle] = model;
	return handle;
}

void ModelBank::LoadMeshes(Model& model, const aiScene* scene) {
	int size = 0;
	int indices = 0;
	Mesh modelMesh;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[i];
		//modelMesh.VertexBufferOffset = size;
		unsigned int numVertices = 0;
		unsigned int numIndices = 0;
		//foreach vertice
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
			glm::vec3 pos, normal, tangent;
			glm::vec2 texcoord;
			pos.x = mesh->mVertices[v].x;
			pos.y = mesh->mVertices[v].y;
			pos.z = mesh->mVertices[v].z;

			model.Max = glm::max(pos, model.Max);
			model.Min = glm::min(pos, model.Min);

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
			
			//m_Vertices.push_back(vertex);
		}//end foreach vertice
		//Indices
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			//index = (Num vertices from the already loaded models) + (Size of all the already loaded meshes + mesh->faceindices)
			m_Indices.push_back(mesh->mFaces[f].mIndices[0]);
			m_Indices.push_back(mesh->mFaces[f].mIndices[1]);
			m_Indices.push_back(mesh->mFaces[f].mIndices[2]);
			numIndices += 3;
		}
		modelMesh.MaterialOffset = mesh->mMaterialIndex;
		modelMesh.IndexBufferOffset = indices;
		size += numVertices;
		indices += numIndices;
		modelMesh.VertexCount = numVertices;
		modelMesh.IndexCount = numIndices;
		model.Meshes.push_back(modelMesh);
	}//end foreach mesh
	model.NumVertices = size;
	model.NumIndices = indices;
}

void ModelBank::LoadRiggedMeshes(Model& model, const aiScene* scene) {
	int vertexCount = 0;
	int indexCount = 0;
	int boneCount = 0;
	Mesh modelMesh;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[i];
		std::vector<AnimatedVertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Bone> bones;
		//modelMesh.VertexBufferOffset = vertexCount;
		unsigned int numVertices = 0;
		unsigned int numIndices = 0;
		//foreach vertice
		for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
			AnimatedVertex vertex;
			vertex.Position.x = mesh->mVertices[v].x;
			vertex.Position.y = mesh->mVertices[v].y;
			vertex.Position.z = mesh->mVertices[v].z;
			if (mesh->HasNormals()) {
				vertex.Normal.x = mesh->mNormals[v].x;
				vertex.Normal.y = mesh->mNormals[v].y;
				vertex.Normal.z = mesh->mNormals[v].z;
			}
			if (mesh->HasTangentsAndBitangents()) {
				vertex.Tangent.x = mesh->mTangents[v].x;
				vertex.Tangent.y = mesh->mTangents[v].y;
				vertex.Tangent.z = mesh->mTangents[v].z;
			}
			if (mesh->mTextureCoords[0] != NULL) {
				vertex.TexCoord.x = mesh->mTextureCoords[0][v].x;
				vertex.TexCoord.y = mesh->mTextureCoords[0][v].y;
			}
			vertex.Weights = glm::vec4(-1);
			vertex.Bones = glm::uvec4(-1);
			numVertices++;
			vertices.push_back(vertex);
		}//end foreach vertice
		 //Indices
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			//index = (Num vertices from the already loaded models) + (Size of all the already loaded meshes + mesh->faceindices)
			indices.push_back(model.VertexHandle + vertexCount + mesh->mFaces[f].mIndices[0]);
			indices.push_back(model.VertexHandle + vertexCount + mesh->mFaces[f].mIndices[1]);
			indices.push_back(model.VertexHandle + vertexCount + mesh->mFaces[f].mIndices[2]);
			numIndices += 3;
		}
		//bones
		Bone meshBone;
		for (unsigned b = 0; b < mesh->mNumBones; b++) {
			aiBone* bone = mesh->mBones[b];
			//add weight to vertices
			for (unsigned w = 0; w < bone->mNumWeights; w++) {
				int id = bone->mWeights[w].mVertexId;
				for (int i = 0; i < 4; i++) {
					if (vertices[id].Weights[i] == -1) {
						vertices[id].Weights[i] = bone->mWeights[w].mWeight;
						vertices[id].Bones[i] = boneCount + b;
						break;
					}
				}
			}
			boneCount++;
			bones.push_back(meshBone);
		}
		modelMesh.MaterialOffset = mesh->mMaterialIndex;
		modelMesh.IndexBufferOffset = indexCount;
		vertexCount += numVertices;
		indexCount += numIndices;
		modelMesh.VertexCount = numVertices;
		modelMesh.IndexCount = numIndices;

		m_AnimatedVertices.insert(m_AnimatedVertices.end(), vertices.begin(), vertices.end());
		m_AnimatedIndices.insert(m_AnimatedIndices.end(), indices.begin(), indices.end());
		model.Meshes.push_back(modelMesh);
	}//end foreach mesh
	model.NumVertices = vertexCount;
	model.NumIndices = indexCount;
}

Skelleton ModelBank::LoadSkelleton(const aiScene* scene) {
	Skelleton skel;
	skel.GlobalInvTransform = glm::inverse(glm::mat4(scene->mRootNode->mTransformation[0][0]));

	for (unsigned m = 0; m < scene->mNumMeshes; m++) {
		aiMesh* mesh = scene->mMeshes[m];
		Bone meshBone;

		for (unsigned b = 0; b < mesh->mNumBones; b++) {
			aiBone* bone = mesh->mBones[b];
			unsigned index;
			std::string name = bone->mName.data;
			if (skel.BoneMapping.find(name) == skel.BoneMapping.end()) {
				index = (unsigned)skel.Bones.size();
				skel.BoneMapping[name] = index;
				meshBone.Name = bone->mName.data;
				meshBone.Offset = glm::mat4(bone->mOffsetMatrix[0][0]);
				skel.Bones.push_back(meshBone);
			}
			else {
				index = skel.BoneMapping[name];
				skel.BoneMapping[name] = index;
				skel.Bones[index].Offset = glm::mat4(bone->mOffsetMatrix[0][0]);
			}
			
		}
	}

	//load node hierarchy
	std::function<void (const aiNode*, glm::mat4&)> readNode = [&](const aiNode* node, glm::mat4& parentTransform ) {
		if (!node)
			return;
		std::string nodeName = node->mName.data;
		const aiAnimation* anim = scene->mAnimations[0];
		glm::mat4 nodeTransform = glm::mat4(node->mTransformation[0][0]);
		int boneIndex = skel.BoneMapping[nodeName];
		Bone* bone = &skel.Bones[boneIndex];
		bone->FinalTransform = skel.GlobalInvTransform * parentTransform * nodeTransform * bone->Offset;
		//find parent index
		if (node->mParent)
			bone->Parent = skel.BoneMapping[node->mParent->mName.data];
		else
			bone->Parent = -1;
		//find children
		for (unsigned c = 0; c < node->mNumChildren; c++) {
			const aiNode* child = node->mChildren[c];
			bone->Children.push_back(skel.BoneMapping[child->mName.data]);
			readNode(child, bone->FinalTransform);
		}
	};

	readNode(scene->mRootNode, glm::mat4(1));
	return skel;

}

void ModelBank::LoadAnimations(Model& model, const aiScene* scene) {
	aiAnimation* animation = scene->mAnimations[0];
	aiNodeAnim* node = animation->mChannels[11];
	int i = 0;
}

ModelHandle ModelBank::AddModel( Model& TheModel ) {
	ModelHandle id = ++m_Numerator;
	//ModelHandle id = ++m_Numerators[TheModel.Type];
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
	//mesh.VertexBufferOffset = 0;
	model.Meshes.push_back(mesh);
	model.MaterialHandle = 0; //make sure there is a default material loaded
	//model.VertexHandle = (int)m_Vertices.size();
	model.IndexHandle = 0;
	model.NumIndices = (unsigned)indices->size();
	model.NumVertices = (unsigned)vertices->size();
	//copy and offset indices
	for (unsigned int i = 0; i < indices->size(); ++i) {
		m_Indices.push_back(model.VertexHandle + indices->at(i));
	}
	//copy vertices
	//m_Vertices.insert(m_Vertices.end(), vertices->begin(), vertices->end());
	m_Models[id] = model;
	return id;
}

void ModelBank::DeleteModel( ) {
	//TODOHJ: Delete model data ,vertices and indices.
	//then update all the other models after it in the memory.
	// tbh its much easier and less cumbersome to just delete all models and load them in again.
}

const Model& ModelBank::FetchModel(ModelHandle handle) {
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

void ModelBank::BuildBuffers() {
	// Vertex buffers
	size_t vboSize = (m_VertexPositions.size() * 4) * (sizeof(glm::vec3) * 3 + sizeof(glm::vec2));

	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vboSize);
	HR(m_Context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_VertexBufferResource)), L"Error creating index buffer resource");
	HR(m_Context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBufferUpload)), L"Error creating index buffer upload resource");
	
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

	//create views for each mesh
	int meshOffset = 0;
	int vec3Size = sizeof(glm::vec3);
	int vec2Size = sizeof(glm::vec2);
	for (auto& model : m_Models) {
		for (auto& mesh : model.second.Meshes) {
			//pos
			mesh.VBOView.PosView.BufferLocation = posHandle + (meshOffset * vec3Size);
			mesh.VBOView.PosView.SizeInBytes = mesh.VertexCount * vec3Size;
			mesh.VBOView.PosView.StrideInBytes = vec3Size;
			//normal
			mesh.VBOView.NormalView.BufferLocation = normalHandle + (meshOffset * vec3Size);
			mesh.VBOView.NormalView.SizeInBytes = mesh.VertexCount * vec3Size;
			mesh.VBOView.NormalView.StrideInBytes = vec3Size;
			//tangent
			mesh.VBOView.TangentView.BufferLocation = tangentHandle + (meshOffset * vec3Size);
			mesh.VBOView.TangentView.SizeInBytes = mesh.VertexCount * vec3Size;
			mesh.VBOView.TangentView.StrideInBytes = vec3Size;
			//texcoord
			mesh.VBOView.TexView.BufferLocation = texHandle + (meshOffset * vec2Size);
			mesh.VBOView.TexView.SizeInBytes = mesh.VertexCount * vec2Size;
			mesh.VBOView.TexView.StrideInBytes = vec2Size;
			meshOffset += mesh.VertexCount;
		}
	}

	m_Context->CommandList->CopyResource(m_VertexBufferResource.Get() , m_VertexBufferUpload.Get());
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBufferResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_Context->CommandList->ResourceBarrier(1, &barrier);
	//Index Buffer
	int staticId = 0;
	for (std::map<ModelHandle, Model>::iterator it = m_Models.begin(); it != m_Models.end(); ++it) {
		it->second.IndexHandle = staticId;
		staticId += it->second.NumIndices;
	}

	size_t indexBufferSize = m_Indices.size() * sizeof(UINT);
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

	HR(m_Context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_IndexBufferResource)), L"Error creating index buffer resource");
	HR(m_Context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexbufferUpload)), L"Error creating index buffer upload resource");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = &m_Indices[0];
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(m_Context->CommandList.Get(), m_IndexBufferResource.Get(), m_IndexbufferUpload.Get(), 0, 0, 1, &indexData);
#ifdef _DEBUG
	m_IndexbufferUpload->SetName(L"Index buffer upload heap");
	m_IndexBufferResource->SetName(L"Index buffer");
#endif

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBufferResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_Context->CommandList->ResourceBarrier(1, &barrier);

	m_IndexBufferView.BufferLocation = m_IndexBufferResource->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = (unsigned)indexBufferSize;
}

void ModelBank::ApplyBuffers(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetIndexBuffer(&m_IndexBufferView);
}

float ModelBank::GetScaledRadius(ModelHandle model, const glm::vec3& scale) {
	float radius = m_Models[model].Radius;
	return radius * glm::max(scale.x, glm::max(scale.y, scale.z));
}

void ModelBank::FreeUploadHeaps(){
	m_IndexbufferUpload.Reset();
	m_VertexBufferUpload.Reset();
}

void ModelBank::UpdateModel(ModelHandle& handle, Model& model) {
	m_Models[handle] = model;
}
