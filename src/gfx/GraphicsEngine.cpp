#include "GraphicsEngine.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
#include "Vertex.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include "BufferManager.h"

GraphicsEngine::GraphicsEngine() {

}

GraphicsEngine::~GraphicsEngine() {
	WaitForGPU(m_Fence, m_Context, m_FrameIndex);
	g_MaterialBank.ClearMaterials();
	g_ModelBank.Clear();
	CloseHandle(m_Fence.FenceEvent);
}

void GraphicsEngine::CreateContext() {

	int renderdoc = 0;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), L"");
	debugController->EnableDebugLayer();
#endif

	HR(CreateDXGIFactory(IID_PPV_ARGS(&m_Context.DXGIFactory)), L"unable to create DXGIFactory");
	HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Context.Device)), L"Error creating device");
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HR(m_Context.Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Context.CommandQueue)), L"Error creating command queue");

	for (int i = 0; i < g_FrameCount; i++) {
		HR(m_Context.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Context.CommandAllocator[i])), L"Error creating command allocator");
	}

	HR(m_Context.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Context.CommandAllocator[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(&m_Context.CommandList)), L"Error creating command list");

	HR(m_Context.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence.Fence)), L"Error creating fence");
	m_Fence.FenceEvent = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
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

	ComPtr<IDXGISwapChain> swapchain;
	HR(m_Context.DXGIFactory->CreateSwapChain(m_Context.CommandQueue.Get(), &swapChainDesc, &swapchain), L"Error creating swapchain");
	swapchain.As(&m_SwapChain.SwapChain);
	m_FrameIndex = m_SwapChain.SwapChain->GetCurrentBackBufferIndex();

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


}

void GraphicsEngine::Init(HWND hWnd, const glm::vec2& screenSize) {
	CreateContext();
	CreateSwapChain(hWnd, screenSize);

	InitGeometryState(&m_ProgramState, &m_Context);
	InitDepthOnlyState(&m_DepthProgramState, &m_Context);

	m_FullscreenPass.Init(&m_Context);
	m_FullscreenPass.CreateSRV(&m_Context, m_DSResource.Get());

	g_BufferManager.Init(&m_Context);
	g_BufferManager.CreateConstBuffer("cbPerFrame", nullptr, sizeof(cbPerFrame));
	g_MaterialBank.Initialize(&m_Context);
	g_ModelBank.Init(&m_Context);
	m_RenderQueue.Init(&m_Context);

	m_Profiler.Init(&m_Context);

	m_Context.CommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_Context.CommandList.Get() };
	m_Context.CommandQueue->ExecuteCommandLists(1, ppCommandList);

	WaitForGPU(m_Fence, m_Context, m_FrameIndex);

	HR(m_Context.CommandAllocator[m_FrameIndex]->Reset(), L"Error resetting command allocator");
	HR(m_Context.CommandList->Reset(m_Context.CommandAllocator[m_FrameIndex].Get(), nullptr), L"Error resetting command list");
}

void GraphicsEngine::ResizeFrameBuffer(const glm::vec2& screenSize) {
	WaitForGPU(m_Fence, m_Context, m_FrameIndex);
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

	WaitForGPU(m_Fence, m_Context, m_FrameIndex);
}

void GraphicsEngine::PrepareForRender() {
	//this will transfer textures/models etc to gpu
	g_ModelBank.BuildBuffers();
	g_MaterialBank.CopyMaterialDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_ProgramState.RenderDescHeap->GetCPUDescriptorHandleForHeapStart()).Offset(3, m_ProgramState.DescHeapIncSize));
	g_MaterialBank.CopyMaterialDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DepthProgramState.MaterialHeap->GetCPUDescriptorHandleForHeapStart()));

	m_Context.CommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_Context.CommandList.Get() };
	m_Context.CommandQueue->ExecuteCommandLists(1, ppCommandList);
	WaitForGPU(m_Fence, m_Context, m_FrameIndex);

	g_MaterialBank.FreeResources();
	g_ModelBank.FreeUploadHeaps();
}

void GraphicsEngine::TransferFrame() {
	HR(m_Context.CommandAllocator[m_FrameIndex]->Reset(), L"Error resetting command allocator");
	HR(m_Context.CommandList->Reset(m_Context.CommandAllocator[m_FrameIndex].Get(), nullptr), L"Error resetting command list");

	m_RenderQueue.UpdateBuffer();

	m_Context.CommandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_Context.CommandList.Get() };
	m_Context.CommandQueue->ExecuteCommandLists(1, ppCommandList);

	WaitForGPU(m_Fence, m_Context, m_FrameIndex);
}

void GraphicsEngine::ClearScreen() {
	//prepare render target for rendering
	m_Context.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_SwapChain.RenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_SwapChain.RenderTargetHeapSize);
	const float clearColor[] = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f };

	m_Context.CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_Context.CommandList->ClearDepthStencilView(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0x0, 0, nullptr);
}

void GraphicsEngine::Render() {
	
	TransferFrame();

	HR(m_Context.CommandAllocator[m_FrameIndex]->Reset(), L"Error resetting command allocator");
	HR(m_Context.CommandList->Reset(m_Context.CommandAllocator[m_FrameIndex].Get(), m_ProgramState.PipelineState.Get()), L"Error resetting command list");

	ClearScreen();

	if (m_RenderQueue.GetDrawCount() == 0) {
		m_Context.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_FrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		HR(m_Context.CommandList->Close(), L"Error closing command list");
		ID3D12CommandList* commandLists = { m_Context.CommandList.Get() };
		m_Context.CommandQueue->ExecuteCommandLists(1, &commandLists);
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_SwapChain.RenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_SwapChain.RenderTargetHeapSize);
	m_Context.CommandList->OMSetRenderTargets(1, &rtvHandle, false, &m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	m_Context.CommandList->RSSetViewports(1, &m_Viewport);
	m_Context.CommandList->RSSetScissorRects(1, &m_ScissorRect);

	View v = m_RenderQueue.GetViews().at(0);
	cbPerFrame* perFrame = (cbPerFrame*)g_BufferManager.MapBuffer("cbPerFrame");
	perFrame->CamPos = v.Camera.Position;
	perFrame->LightDir = glm::vec3(0.0f, -1.0f, 0.2f);
	perFrame->ViewProj = v.Camera.ProjView;
	g_BufferManager.UnMapBuffer("cbPerFrame");

	m_Profiler.Step(m_Context.CommandList.Get(), "Pre-Z");

	DepthOnlyRender(m_Context.CommandList.Get(), &m_DepthProgramState, &m_RenderQueue);

	m_Profiler.Step(m_Context.CommandList.Get(), "Geometry");

	RenderGeometry(m_Context.CommandList.Get(), &m_ProgramState, &m_RenderQueue);

	m_Profiler.End(m_Context.CommandList.Get());

	//return to present mode for render target
	m_Context.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain.RenderTargets[m_FrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	HR(m_Context.CommandList->Close(),L"Error closing command list");
	ID3D12CommandList* commandLists = { m_Context.CommandList.Get() };
	m_Context.CommandQueue->ExecuteCommandLists(1, &commandLists);

	m_Profiler.PrintResults();
}

void GraphicsEngine::Swap() {
	m_SwapChain.SwapChain->Present(1, 0);
	m_RenderQueue.Clear();
	const UINT64 currentFenceValue = m_Fence.FenceValues[m_FrameIndex];
	m_Context.CommandQueue->Signal(m_Fence.Fence.Get(), m_Fence.FenceValues[m_FrameIndex]);

	m_FrameIndex = m_SwapChain.SwapChain->GetCurrentBackBufferIndex();

	if (m_Fence.Fence->GetCompletedValue() < m_Fence.FenceValues[m_FrameIndex]) {
		m_Fence.Fence->SetEventOnCompletion(m_Fence.FenceValues[m_FrameIndex], m_Fence.FenceEvent);
		WaitForSingleObjectEx(m_Fence.FenceEvent, INFINITE, false);
	}
	m_Fence.FenceValues[m_FrameIndex] = currentFenceValue + 1;
}