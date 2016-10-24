#include "VertexBuffer.h"

VertexBuffer::VertexBuffer() {

}

VertexBuffer::~VertexBuffer() {

}

void VertexBuffer::Init(const std::vector<Vertex>& vertices, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
	size_t bufferSize = vertices.size() * sizeof(Vertex);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_VBO)), L"Error creating vertex buffer resource");
	HR(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadHeap)), L"Error creating vertex buffer upload resource");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &vertices[0];
	vertexData.RowPitch = bufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(cmdList, m_VBO.Get(), m_UploadHeap ,0, 0, 1, &vertexData);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_VBO.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	cmdList->ResourceBarrier(1, &barrier);

	m_View.BufferLocation = m_VBO->GetGPUVirtualAddress();
	m_View.StrideInBytes = sizeof(Vertex);
	m_View.SizeInBytes = bufferSize;

#ifdef _DEBUG
	m_UploadHeap->SetName(L"Vertex Buffer Upload");
	m_VBO->SetName(L"Vertex Buffer");
#endif
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetView() {
	return m_View;
}

void VertexBuffer::FreeUploadHeap(){
	m_UploadHeap->Release();
}

void VertexBuffer::Release() {
	if(m_VBO != NULL)
		m_VBO.Get()->Release();
}