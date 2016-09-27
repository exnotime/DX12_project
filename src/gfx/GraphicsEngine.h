#pragma once
#include "Common.h"
#include "Shader.h"
#include "RenderQueue.h"
#include "Texture.h"
#include "GeometryProgram.h"
#include "DepthOnlyProgram.h"
#include "GPUProfiler.h"
#include "FullscreenPass.h"
#include "HiZProgram.h"

#define SIGNAL_BEGIN_COPY 0
#define SIGNAL_END_COPY 1

struct cbPerFrame {
	glm::mat4 ViewProj;
	glm::vec3 CamPos;
	float padding;
	glm::vec3 LightDir;
	float padding2;
};

class GraphicsEngine {
public:
	GraphicsEngine();
	~GraphicsEngine();

	void Init(HWND hWnd, const glm::vec2& screenSize);
	void ResizeFrameBuffer(const glm::vec2& screenSize);
	void PrepareForRender();

	void TransferFrame();
	void ClearScreen();
	void Render();
	void Swap();

	RenderQueue* GetRenderQueue() { return &m_RenderQueue; }
private:
	void CreateContext();
	void CreateSwapChain(HWND hWnd, const glm::vec2& screenSize);

	glm::vec2 m_ScreenSize;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;
	UINT m_FrameIndex = 0;
	UINT m_HeapIncrementSize = 0;

	DX12Context m_Context;
	DX12Fence m_Fence;
	DX12SwapChain m_SwapChain;
	//depth/stencil
	ComPtr<ID3D12Resource> m_DSResource;
	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	RenderQueue m_RenderQueue;
	GeometryProgram::GeometryProgramState m_ProgramState;
	DepthOnlyProgram m_DepthProgram;
	GPUProfiler m_Profiler;

	FullscreenPass m_FullscreenPass;
	HiZProgram m_HiZProgram;

};