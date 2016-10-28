#include "GraphicsEngine.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "BufferManager.h"
#include "TestParams.h"
#include "CommandBufferManager.h"

//shader extensions
#include <amd_ags.h>
#include <NVAPI/nvapi.h>
#include <NVAPI/nvShaderExtnEnums.h>
GraphicsEngine::GraphicsEngine() {

}

GraphicsEngine::~GraphicsEngine() {
	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);
	g_MaterialBank.ClearMaterials();
	g_ModelBank.Clear();
	CloseHandle(m_Fence.FenceEvent);

	if (m_Context.Extensions.Vendor == AMD_VENDOR_ID) {
		agsDeInit(m_Context.Extensions.AGSContext);
	}
	else if (m_Context.Extensions.Vendor == NVIDIA_VENDOR_ID) {
		NvAPI_Unload();
	}
}

void GraphicsEngine::CreateExtensionContext() {
	//find out if we run on Nvidia or AMD
	IDXGIAdapter* adapter;
	DXGI_ADAPTER_DESC adapterInfo;
	m_Context.DXGIFactory->EnumAdapters(0, &adapter);
	adapter->GetDesc(&adapterInfo);

	if (adapterInfo.VendorId == AMD_VENDOR_ID) {
		//set up AGS
		AGSContext* context;
		if (agsInit(&context, nullptr, nullptr) == AGS_SUCCESS) {
			m_Context.Extensions.Vendor = AMD_VENDOR_ID;
			m_Context.Extensions.AGSContext = context;
			m_Context.Extensions.WaveSize = 64;
		}
	} else if (adapterInfo.VendorId == NVIDIA_VENDOR_ID) {
		//set up NVAPI
		if (NvAPI_Initialize() != NVAPI_OK) {
			HR(E_FAIL, L"This computer can not run this program");
			exit(0);
		}
		m_Context.Extensions.Vendor = NVIDIA_VENDOR_ID;
		m_Context.Extensions.AGSContext = nullptr;
		m_Context.Extensions.WaveSize = 32;
	} else {
		//Cant run this T.T
		HR(E_FAIL, L"This computer can not run this program");
		exit(0);
	}
}

void GraphicsEngine::CheckExtensions() {
	//set up extensions
	if (m_Context.Extensions.Vendor == AMD_VENDOR_ID) {
		UINT extensions = 0;
		if (agsDriverExtensionsDX12_Init(m_Context.Extensions.AGSContext, m_Context.Device.Get(), &extensions) == AGS_SUCCESS) {
			//check so that we support the extensions we want
			UINT extensionsWeWant = AGS_DX12_EXTENSION_INTRINSIC_BALLOT | AGS_DX12_EXTENSION_INTRINSIC_LANEID |
				AGS_DX12_EXTENSION_INTRINSIC_MBCOUNT | AGS_DX12_EXTENSION_INTRINSIC_READFIRSTLANE |
				AGS_DX12_EXTENSION_INTRINSIC_READLANE | AGS_DX12_EXTENSION_INTRINSIC_SWIZZLE;

			if (!(extensions & extensionsWeWant)) {
				HR(E_FAIL, L"The GPU doesn not support the extentions this application needs");
				exit(0);
			}
		}
	} else if (m_Context.Extensions.Vendor == NVIDIA_VENDOR_ID) {
		bool supported;
		NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(m_Context.Device.Get(), NV_EXTN_OP_VOTE_BALLOT, &supported);
		if(!supported) {
			HR(E_FAIL, L"The GPU doesn not support the extentions this application needs");
			exit(0);
		}
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 8, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		m_Context.Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_Context.Extensions.NvExtResource));
	}
}

void GraphicsEngine::CreateContext() {
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), L"debug controller");
	debugController->EnableDebugLayer();
#endif

	HR(CreateDXGIFactory(IID_PPV_ARGS(&m_Context.DXGIFactory)), L"unable to create DXGIFactory");
	CreateExtensionContext();

	HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Context.Device)), L"Error creating device");
	CheckExtensions();
	//Graphics Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HR(m_Context.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Context.CommandQueue)), L"Error creating command queue");
	//Copy Queue
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	HR(m_Context.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Context.CopyQueue)), L"Error creating copy queue");
	//Compute Queue
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	HR(m_Context.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Context.ComputeQueue)), L"Error creating compute queue");

	g_CommandBufferManager.Init(&m_Context, 16, 16, 1);
	g_CommandBufferManager.ResetAllCommandBuffers();

	HR(m_Context.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence.Fence)), L"Error creating fence");
	HR(m_Context.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Context.CopyFence)), L"Error creating fence");
	m_Fence.FenceEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
	for (int i = 0; i < g_FrameCount; ++i) {
		m_Fence.FenceValues[i] = 0;
	}
}

void GraphicsEngine::CreateSwapChain(HWND hWnd, const glm::vec2& screenSize) {
	m_ScreenSize = screenSize * 1.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.Width = m_ScreenSize.x;
	m_Viewport.Height = m_ScreenSize.y;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.bottom = (unsigned)m_ScreenSize.y;
	m_ScissorRect.right = (unsigned)m_ScreenSize.x;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = g_FrameCount;
	swapChainDesc.BufferDesc.Width = (unsigned)m_ScreenSize.x;
	swapChainDesc.BufferDesc.Height = (unsigned)m_ScreenSize.y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	ComPtr<IDXGISwapChain> swapchain;
	HR(m_Context.DXGIFactory->CreateSwapChain(m_Context.CommandQueue.Get(), &swapChainDesc, &swapchain), L"Error creating swapchain");
	swapchain.As(&m_SwapChain.SwapChain);
	m_Context.FrameIndex = m_SwapChain.SwapChain->GetCurrentBackBufferIndex();

	m_SwapChain.RenderTargets;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = g_FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(m_Context.Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_SwapChain.RenderTargetDescHeap)), L"Error creating descriptor heap for render targets");

	D3D12_RESOURCE_DESC dsvResDesc = {};
	dsvResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsvResDesc.Width = (unsigned)m_ScreenSize.x;
	dsvResDesc.Height = (unsigned)m_ScreenSize.y;
	dsvResDesc.DepthOrArraySize = 1;
	dsvResDesc.MipLevels = 1;
	dsvResDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsvResDesc.SampleDesc.Count = 1;
	dsvResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsvResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.DepthStencil.Depth = 1.0f;
	clearVal.DepthStencil.Stencil = 0x0;
	clearVal.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	HR(m_Context.Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&dsvResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal, IID_PPV_ARGS(&m_DSResource)), L"Error creating depthstencil resource");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HR(m_Context.Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)), L"Error creating descriptor heap for DSV");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	m_Context.Device->CreateDepthStencilView(m_DSResource.Get(), &dsvDesc, dsvHandle);

	m_SwapChain.RenderTargetHeapSize = m_Context.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE  rtvHandle(m_SwapChain.RenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < g_FrameCount; i++) {
		m_SwapChain.SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChain.RenderTargets[i]));
		m_Context.Device->CreateRenderTargetView(m_SwapChain.RenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_SwapChain.RenderTargetHeapSize); //offset with size
	}
#ifdef _DEBUG
	m_SwapChain.RenderTargets[0]->SetName(L"RenderTarget0");
	m_SwapChain.RenderTargets[1]->SetName(L"RenderTarget1");
#endif
}

void GraphicsEngine::Init(HWND hWnd, const glm::vec2& screenSize) {
	CreateContext();
	CreateSwapChain(hWnd, screenSize);

	m_DepthProgram.Init(&m_Context, m_ScreenSize);

	m_HiZProgram.Init(&m_Context, m_ScreenSize);
	m_FullscreenPass.Init(&m_Context);
	m_FullscreenPass.CreateSRV(&m_Context, m_HiZProgram.GetResource(), DXGI_FORMAT_R32_FLOAT, m_HiZProgram.GetMipCount());
	
	g_BufferManager.Init(m_Context.Device.Get());
	g_BufferManager.CreateConstBuffer("cbPerFrame", sizeof(cbPerFrame));
	g_BufferManager.CreateConstBuffer("cbPerFrame2", sizeof(cbPerFrame));
	g_BufferManager.CreateConstBuffer("testBuffer", sizeof(IndirectDrawCall) * 100);

	ID3D12GraphicsCommandList* cmdList = g_CommandBufferManager.GetNextCommandList(GRAPHICS_TYPE); //cmd is closed when we get it here!!
	InitGeometryState(&m_ProgramState, &m_Context, cmdList);
	g_MaterialBank.Initialize(m_Context.Device.Get(), cmdList);
	g_ModelBank.Init();
	m_RenderQueue.Init();

	m_FilterContext.Init(m_Context.Device.Get());
	m_TriangleCullingProgram.Init(&m_Context);
	m_CullingProgram.Init(&m_Context, &m_FilterContext);

	m_Profiler.Init(&m_Context);

	g_CommandBufferManager.ExecuteCommandBuffers(GRAPHICS_TYPE);

	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);

	g_CommandBufferManager.ResetAllCommandBuffers();
}

void GraphicsEngine::ResizeFrameBuffer(const glm::vec2& screenSize) {
	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);
	//get window 
	HWND hWnd;
	m_SwapChain.SwapChain->GetHwnd(&hWnd);
	//destroy swapchain
	m_SwapChain.SwapChain.Reset();
	m_SwapChain.RenderTargets[0].Reset();
	m_SwapChain.RenderTargets[1].Reset();
	m_DSResource.Reset();
	m_DSVHeap.Reset();
	//create swapchain
	CreateSwapChain(hWnd, screenSize);
	m_ScreenSize = screenSize;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.Width = screenSize.x;
	m_Viewport.Height = screenSize.y;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.bottom = (unsigned)screenSize.y;
	m_ScissorRect.right = (unsigned)screenSize.x;

	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);
}

void GraphicsEngine::PrepareForRender() {

	ID3D12GraphicsCommandList* cmdList = g_CommandBufferManager.GetNextCommandList(GRAPHICS_TYPE);
	//this will transfer textures/models etc to gpu
	g_ModelBank.BuildBuffers(m_Context.Device.Get(), cmdList);
	g_MaterialBank.CopyMaterialDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_ProgramState.RenderDescHeap->GetCPUDescriptorHandleForHeapStart()).Offset(3, m_ProgramState.DescHeapIncSize));

	g_CommandBufferManager.ExecuteCommandBuffers(GRAPHICS_TYPE);

	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);

	m_TriangleCullingProgram.CreateDescriptorTable(&m_HiZProgram);

	g_MaterialBank.FreeResources();
	g_ModelBank.FreeUploadHeaps();
}

void GraphicsEngine::TransferFrame() {
	
	ID3D12GraphicsCommandList* cmdList = g_CommandBufferManager.GetNextCommandList(GRAPHICS_TYPE);

	m_RenderQueue.UpdateBuffer(cmdList);
	g_CommandBufferManager.ExecuteCommandBuffers(GRAPHICS_TYPE);
	WaitForGPU(m_Fence, m_Context, m_Context.FrameIndex);
}

void GraphicsEngine::ClearScreen(ID3D12GraphicsCommandList* cmdList) {
	//prepare render target for rendering
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_Context.FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_SwapChain.RenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart(), m_Context.FrameIndex, m_SwapChain.RenderTargetHeapSize);
	const float clearColor[] = { 0,0,0,0 };

	cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0x0, 0, nullptr);
}

void GraphicsEngine::Render() {
	//culling test stuff
	if (g_TestParams.Reset) {
		m_Profiler.Reset();
		m_FilterContext.Reset(m_Context.Device.Get());
		m_TriangleCullingProgram.Reset(&m_Context);
		//reset the filter context and triangle culling program as well

		g_TestParams.Reset = false;
	}
	g_CommandBufferManager.ResetAllCommandBuffers();
	TransferFrame();

	ID3D12GraphicsCommandList* cmdList = g_CommandBufferManager.GetNextCommandList(GRAPHICS_TYPE);

	ClearScreen(cmdList);

	if (m_RenderQueue.GetDrawCount() == 0) {
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_Context.FrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		g_CommandBufferManager.ExecuteCommandBuffers(GRAPHICS_TYPE);
		return;
	}

	m_FilterContext.Clear(cmdList);
	m_CullingProgram.ClearCounters(cmdList);

	View v = m_RenderQueue.GetViews().at(1);
	cbPerFrame* perFrame = (cbPerFrame*)g_BufferManager.MapBuffer("cbPerFrame");
	perFrame->ViewProj = v.Camera.ProjView;
	perFrame->ScreenSize = m_ScreenSize;
	g_BufferManager.UnMapBuffer("cbPerFrame");
	m_Profiler.Start();
	v = m_RenderQueue.GetViews().at(0);
	perFrame = (cbPerFrame*)g_BufferManager.MapBuffer("cbPerFrame2");
	perFrame->CamPos = v.Camera.Position;
	perFrame->LightDir = glm::vec3(0.0f, -1.0f, 0.2f);
	perFrame->ViewProj = v.Camera.ProjView;
	g_BufferManager.UnMapBuffer("cbPerFrame2");

	//m_Profiler.Step(cmdList, "DepthRender");
	//m_DepthProgram.Render(cmdList, &m_RenderQueue);

	//m_Profiler.Step(cmdList, "Hi-z");
	//m_HiZProgram.Disbatch(cmdList, m_DepthProgram.GetDepthTexture());

	SetRenderTarget(cmdList);
	if (g_TestParams.UseCulling) {
		m_Profiler.Step(cmdList, "TriangleFilter");
		while (m_TriangleCullingProgram.Disbatch(cmdList, &m_RenderQueue, &m_FilterContext)) {
			m_Profiler.Step(cmdList, "DrawCulling");
			m_CullingProgram.Disbatch(cmdList, &m_FilterContext);
			m_Profiler.Step(cmdList, "Render");
			RenderGeometry(cmdList, &m_ProgramState, &m_FilterContext, &m_CullingProgram);
			m_Profiler.Step(cmdList, "TriangleFilter");
		}

		m_Profiler.Step(cmdList, "DrawCulling");
		m_CullingProgram.Disbatch(cmdList, &m_FilterContext);
		m_Profiler.Step(cmdList, "Render");
		RenderGeometry(cmdList, &m_ProgramState, &m_FilterContext, &m_CullingProgram);
	}
	else {
		m_Profiler.Step(cmdList, "Render");
		RenderGeometryWithoutCulling(cmdList, &m_ProgramState, &m_RenderQueue);
	}

	m_Profiler.End(cmdList, m_Context.FrameIndex);
	//return to present mode for render target
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_Context.FrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	g_CommandBufferManager.ExecuteCommandBuffers(GRAPHICS_TYPE);
}

void GraphicsEngine::Swap() {
	//swap and clear
	m_SwapChain.SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	m_RenderQueue.Clear();
	//signal when this frame is done
	const UINT64 currentFenceValue = m_Fence.FenceValues[m_Context.FrameIndex];
	m_Context.CommandQueue->Signal(m_Fence.Fence.Get(), m_Fence.FenceValues[m_Context.FrameIndex]);

	//wait for last frame to finish
	m_Context.FrameIndex = m_SwapChain.SwapChain->GetCurrentBackBufferIndex();

	if (m_Fence.Fence->GetCompletedValue() < m_Fence.FenceValues[m_Context.FrameIndex]) {
		m_Fence.Fence->SetEventOnCompletion(m_Fence.FenceValues[m_Context.FrameIndex], m_Fence.FenceEvent);
		WaitForSingleObjectEx(m_Fence.FenceEvent, INFINITE, false);
	}
	m_Fence.FenceValues[m_Context.FrameIndex] = currentFenceValue + 1;
	m_Profiler.PrintResults(m_Context.FrameIndex);
	
}

void GraphicsEngine::SetRenderTarget(ID3D12GraphicsCommandList* cmdList) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_SwapChain.RenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart(), m_Context.FrameIndex, m_SwapChain.RenderTargetHeapSize);
	cmdList->OMSetRenderTargets(1, &rtvHandle, false, &m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	cmdList->RSSetViewports(1, &m_Viewport);
	cmdList->RSSetScissorRects(1, &m_ScissorRect);
}