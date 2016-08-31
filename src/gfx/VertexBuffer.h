#pragma once
#include "Common.h"
#include "Vertex.h"
class VertexBuffer {
public:
	VertexBuffer();
	~VertexBuffer();
	void Init(const std::vector<Vertex>& vertices, DX12Context context);
	D3D12_VERTEX_BUFFER_VIEW GetView();
	void FreeUploadHeap();
	void Release();
private:
	ComPtr<ID3D12Resource> m_VBO;
	ID3D12Resource* m_UploadHeap;
	D3D12_VERTEX_BUFFER_VIEW m_View;
};