#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <DirectXTex/DirectXTex.h>
Texture::Texture() {

}
Texture::~Texture() {
	m_Resource.Reset();
}

bool Texture::Init(const std::string& filename, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
	if (strstr(filename.c_str(), ".dds")) {
		DirectX::ScratchImage scratchImage;
		DirectX::TexMetadata meta;
		wchar_t* name = convertCharArrayToLPCWSTR(filename.c_str());
		HR(DirectX::LoadFromDDSFile(name, 0, &meta, scratchImage), L"Error loading texture");
		delete name;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.MipLevels = meta.mipLevels;
		texDesc.Width = meta.width;
		texDesc.Height = meta.height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.DepthOrArraySize = meta.arraySize;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = meta.format;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		m_IsCubeMap = meta.IsCubemap();
		m_Miplevels = meta.mipLevels;
		m_Width = meta.width;
		m_Height = meta.height;
		m_Format = meta.format;
		HRESULT hr;
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_Resource));

		if (hr != S_OK) {
			return false;
		}

		const UINT64 uploadSize = GetRequiredIntermediateSize(m_Resource.Get(), 0, meta.mipLevels * meta.arraySize);

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadHeap));
		if (hr != S_OK) {
			return false;
		}

		D3D12_SUBRESOURCE_DATA* texDataArray = new D3D12_SUBRESOURCE_DATA[meta.mipLevels * meta.arraySize];
		for (int side = 0; side < meta.arraySize; side++) {
			for (int mip = 0; mip < meta.mipLevels; mip++) {
				const DirectX::Image* image = scratchImage.GetImage(mip, side, 0);
				int index = (side * meta.mipLevels) + mip;
				texDataArray[index].pData = (void*)image->pixels;
				texDataArray[index].RowPitch = image->rowPitch;
				texDataArray[index].SlicePitch = image->slicePitch;
			}
		}
		int size = UpdateSubresources(cmdList, m_Resource.Get(), m_UploadHeap.Get(), 0, 0, meta.mipLevels * meta.arraySize, texDataArray);

		delete [] texDataArray;
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
#ifdef _DEBUG
		wchar_t* wname = convertCharArrayToLPCWSTR(filename.c_str());
		m_Resource->SetName(wname);
		delete[] wname;
		m_UploadHeap->SetName(L"upload heap");
#endif
		printf("Finished loading texture: %s\n", filename.c_str());
		return true;
	}

	unsigned char* textureData = stbi_load(filename.c_str(), &m_Width, &m_Height, &m_Channels, 4);
	if (textureData) {
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.MipLevels = 1;
		texDesc.Width = m_Width;
		texDesc.Height = m_Height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.DepthOrArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		
		m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		HRESULT hr;
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_Resource));

		if (hr != S_OK) {
			return false;
		}

		const UINT64 uploadSize = GetRequiredIntermediateSize(m_Resource.Get(), 0, 1);

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadHeap));
		if (hr != S_OK) {
			return false;
		}
		D3D12_SUBRESOURCE_DATA texData = {};
		texData.pData = (void*)textureData;
		texData.RowPitch = m_Width * 4;
		texData.SlicePitch = texData.RowPitch * m_Height;

		UpdateSubresources(cmdList, m_Resource.Get(), m_UploadHeap.Get(), 0, 0, 1, &texData);

		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
#ifdef _DEBUG
		wchar_t* name = convertCharArrayToLPCWSTR(filename.c_str());
		m_Resource->SetName(name);
		delete[] name;
		m_UploadHeap->SetName(L"upload heap");
#endif
		printf("Finished loading texture: %s\n", filename.c_str());
		stbi_image_free(textureData);
	}
	else {
		printf("!!!Error loading texture!!!: %s\n", filename.c_str());
		return false;
	}

	return true;
}

void Texture::CreateSRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE handle) {

	D3D12_SHADER_RESOURCE_VIEW_DESC srvView = {};
	srvView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvView.Format = m_Format;
	srvView.Texture2D.MipLevels = m_Miplevels;
	if (m_IsCubeMap) {
		srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvView.TextureCube.MipLevels = m_Miplevels;
	}
	else {
		srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvView.Texture2D.MipLevels = m_Miplevels;
	}
	
	device->CreateShaderResourceView(m_Resource.Get(), &srvView, handle);
}

void Texture::FreeUploadHeap() {
	m_UploadHeap.Reset();
}

D3D12_GPU_VIRTUAL_ADDRESS Texture::GetAddress() {
	return m_Resource->GetGPUVirtualAddress();
}
