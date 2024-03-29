#pragma once
#include <glm/glm.hpp>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_5.h>
#include <wrl/client.h>
#include <vector>
#include <string>

#include <amd_ags.h>
#define AMD_VENDOR_ID 4098
#define NVIDIA_VENDOR_ID 4318
#define NVIDIA_EXTENSION_SPACE 10
#define NVIDIA_EXTENSION_SLOT 1

using Microsoft::WRL::ComPtr;
#define HR(x,errorstring) if(x != S_OK) {MessageBox(NULL, errorstring, L"DX12Error", MB_OK);}
static const UINT g_FrameCount = 2;

struct ExtensionContext {
	union {
		AGSContext* AGSContext; //context for all ags functions
		
	};
	ComPtr<ID3D12Resource> NvExtResource; //resource to contain the uav needed for nvidia extentions
	int Vendor;
	int WaveSize;
};

struct DX12Context {
	ComPtr<ID3D12Device> Device;

	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12CommandQueue> CopyQueue;
	ComPtr<ID3D12CommandQueue> ComputeQueue;

	//ComPtr<ID3D12CommandAllocator> CommandAllocator[g_FrameCount];
	//ComPtr<ID3D12CommandAllocator> CopyAllocator[g_FrameCount];
	//ComPtr<ID3D12CommandAllocator> ComputeAllocator[g_FrameCount];

	//ComPtr<ID3D12GraphicsCommandList> CommandList;
	//ComPtr<ID3D12GraphicsCommandList> CopyCommandList;
	//ComPtr<ID3D12GraphicsCommandList> ComputeCommandList;

	ComPtr<ID3D12Fence> CopyFence;
	ComPtr<IDXGIFactory> DXGIFactory;
	ExtensionContext Extensions;
	UINT FrameIndex = 0;
};

struct DX12SwapChain {
	ComPtr<IDXGISwapChain3> SwapChain;
	ComPtr<ID3D12Resource> RenderTargets[g_FrameCount];
	ComPtr<ID3D12DescriptorHeap> RenderTargetDescHeap;
	UINT RenderTargetHeapSize;
};

struct DX12Fence {
	ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValues[g_FrameCount];
	HANDLE FenceEvent;
};

static void WaitForGPU(DX12Fence& fence, DX12Context context, UINT frameIndex) {
	context.CommandQueue->Signal(fence.Fence.Get(), fence.FenceValues[frameIndex]);
	fence.Fence->SetEventOnCompletion(fence.FenceValues[frameIndex], fence.FenceEvent);
	WaitForSingleObjectEx(fence.FenceEvent, INFINITE,false);
	fence.FenceValues[frameIndex]++;
}

static wchar_t* convertCharArrayToLPCWSTR(const char* charArray) {
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}