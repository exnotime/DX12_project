#pragma once
#include "DX12Common.h"
class Texture {
public:
	Texture();
	~Texture();
	bool Init(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void CreateSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE handle);
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