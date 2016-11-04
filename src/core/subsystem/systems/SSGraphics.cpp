#include "SSGraphics.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <gfx/ModelBank.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "../../datasystem/ComponentManager.h"
#include "../../Window.h"
#include "../../components/CameraComponent.h"
#include "../../components/TransformComponent.h"
#include "../../components/ModelComponent.h"
#include "../../entity/EntityManager.h"
#include "../../entity/EntityFactory.h"
#include "../../input/Input.h"
#include <gfx/TestParams.h>
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

void SSGraphics::Update(const double deltaTime) {
	static bool firstUpdate = true;

	int flag = CameraComponent::Flag;
	static CameraData lastCam;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);
			cc->Camera.CalculateViewProjection();
			View v;
			v.Camera = cc->Camera.GetData();
			m_RenderQueue->AddView(v);

			//if (g_Input.IsKeyPushed(GLFW_KEY_T) || g_Input.IsMousebuttonDown(GLFW_MOUSE_BUTTON_RIGHT) || firstUpdate || g_TestParams.CurrentTest.Culling)
				lastCam = cc->Camera.GetData();
			v.Camera = lastCam;
			m_RenderQueue->AddView(v);
		}
	}
	if (firstUpdate) {
		m_Graphics->PrepareForRender();
		firstUpdate = false;
	}
	ShaderInput si;
	flag = TransformComponent::Flag | ModelComponent::Flag;
	for ( const auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)g_ComponentManager.GetComponent(entity, ModelComponent::Flag);
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(entity, TransformComponent::Flag);

			si.Color = mc->Color;
			si.World = glm::translate(tc->Position) * glm::mat4_cast(tc->Orientation) * glm::scale(tc->Scale);
			m_RenderQueue->Enqueue(mc->Model, si);

			if (mc->Occluder != -1) {
				m_RenderQueue->EnqueueOccluder(mc->Occluder);
			}
		}
	}
	m_Graphics->Render();
	m_Graphics->Swap();
}

void SSGraphics::Shutdown() {
	delete m_Graphics;
}