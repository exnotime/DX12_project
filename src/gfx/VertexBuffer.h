#pragma once
#include "DX12Common.h"
#include "Vertex.h"
class VertexBuffer {
public:
	VertexBuffer();
	~VertexBuffer();
	void Init(const std::vector<Vertex>& vertices,ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	D3D12_VERTEX_BUFFER_VIEW GetView();
	void FreeUploadHeap();
	void Release();
private:
	ComPtr<ID3D12Resource> m_VBO;
	ID3D12Resource* m_UploadHeap;
	D3D12_VERTEX_BUFFER_VIEW m_View;
};