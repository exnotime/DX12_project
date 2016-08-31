#include "SSGraphics.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "datasystem/ComponentManager.h"
#include "../../Window.h"
#include "../core/components/CameraComponent.h"
#include "../core/components/TransformComponent.h"
#include "../core/components/ModelComponent.h"
#include "../core/entity/EntityManager.h"
#include "../core/entity/EntityFactory.h"
#include "../../../gfx/ModelBank.h"
#include <glm/gtx/transform.hpp>

//threading test TODO: REMOVE
#include "../core/threading/JobManager.h"
#include <sstream>


SSGraphics::SSGraphics(){

}

SSGraphics::~SSGraphics(){

}

void SSGraphics::Startup() {
	HWND hWnd = glfwGetWin32Window(g_Window.GetWindow());
	WindowSettings ws = g_Window.GetWindowSettings();
	if (hWnd) {
		m_Graphics = new GraphicsEngine();
		m_Graphics->Init(hWnd, glm::vec2(ws.Width, ws.Height));
		m_RenderQueue = m_Graphics->GetRenderQueue();
	}
}

void SSGraphics::Update(const float deltaTime) {
	static bool firstUpdate = true;
	if (firstUpdate) {
		m_Graphics->PrepareForRender();
		firstUpdate = false;
	}
	int flag = CameraComponent::Flag;
	CameraData cd;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);
			cc->Camera.CalculateViewProjection();
			View v;
			v.Camera = cc->Camera.GetData();
			m_RenderQueue->AddView(v);
		}
	}
	ShaderInput si;
	flag = TransformComponent::Flag | ModelComponent::Flag;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)g_ComponentManager.GetComponent(entity, ModelComponent::Flag);
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(entity, TransformComponent::Flag);

			si.Color = mc->Color;
			si.World = glm::translate(tc->Position) * glm::mat4_cast(tc->Orientation) * glm::scale(tc->Scale);
			m_RenderQueue->Enqueue(mc->Model, si);
		}
	}
	m_Graphics->Render();
	m_Graphics->Swap();
}

void SSGraphics::Shutdown() {
	delete m_Graphics;
}
