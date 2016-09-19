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
#include <glm/gtc/matrix_access.hpp>

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
	if (firstUpdate) {
		m_Graphics->PrepareForRender();
		firstUpdate = false;
	}

	int flag = CameraComponent::Flag;
	glm::vec4 frustumPlanes[6];
	glm::vec3 camPos;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);
			cc->Camera.CalculateViewProjection();
			View v;
			v.Camera = cc->Camera.GetData();
			m_RenderQueue->AddView(v);

			glm::mat4 vp = cc->Camera.GetData().ProjView;
			glm::vec4 row1 = glm::row(vp, 0);
			glm::vec4 row2 = glm::row(vp, 1);
			glm::vec4 row3 = glm::row(vp, 2);
			glm::vec4 row4 = glm::row(vp, 3);
			frustumPlanes[0] = row4 + row1;
			frustumPlanes[1] = row4 - row1;
			frustumPlanes[2] = row4 + row2;
			frustumPlanes[3] = row4 - row2;
			frustumPlanes[4] = row4 + row3;
			frustumPlanes[5] = row4 - row3;
			for (int i = 0; i < 6; ++i) {
				frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
			}
			camPos = cc->Camera.GetData().Position;
		}
	}
	ShaderInput si;
	flag = TransformComponent::Flag | ModelComponent::Flag;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)g_ComponentManager.GetComponent(entity, ModelComponent::Flag);
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(entity, TransformComponent::Flag);

			float r = g_ModelBank.GetScaledRadius(mc->Model, tc->Scale);
			//if (FrustumCheck(frustumPlanes, tc->Position - camPos, 1)) {
				si.Color = mc->Color;
				si.World = glm::translate(tc->Position) * glm::mat4_cast(tc->Orientation) * glm::scale(tc->Scale);
				m_RenderQueue->Enqueue(mc->Model, si);
			//}
		}
	}
	m_Graphics->Render();
	m_Graphics->Swap();
}

void SSGraphics::Shutdown() {
	delete m_Graphics;
}

bool SSGraphics::FrustumCheck(const glm::vec4* planes, const glm::vec3& pos, float radius) {
	for (int i = 0; i < 6; ++i) {
		float d = glm::dot(glm::vec4(pos,1), planes[i]);
		if (d < -radius) {
			return false;
		}
	}
	return true;
}