#pragma once
#include "DX12Common.h"
class Texture {
public:
	Texture();
	~Texture();
	void Init(const std::string& filename, DX12Context* context);
	void CreateSRV(DX12Context* context, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void FreeUploadHeap();
	ID3D12Resource* GetResource() { return m_Resource.Get(); };
	D3D12_GPU_VIRTUAL_ADDRESS GetAddress();
private:
	int m_Width;
	int m_Height;
	int m_Channels;
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<ID3D12Resource> m_UploadHeap;
	bool m_IsCubeMap = false;
	UINT m_Miplevels = 1;
	DXGI_FORMAT m_Format;
};