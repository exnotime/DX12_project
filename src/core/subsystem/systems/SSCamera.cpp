#include "SSCamera.h"
#include "../../datasystem/ComponentManager.h"
#include "../../entity/EntityManager.h"
#include "../../components/CameraComponent.h"
#include "../../Input/Input.h"
#include "../../entity/EntityFactory.h"
#include <gfx/TestParams.h>

#define MOVE_SPEED 10.0f
#define SPEED_UP 4.0f
#define SPEED_DOWN 0.25f
#define TURN_SPEED 0.004f
SSCamera::SSCamera() {

}

SSCamera::~SSCamera() {

}

void SSCamera::Startup() {
	
}

void SSCamera::Update(const double deltaTime) {
#ifdef FREE_CAMERA
	int flag = CameraComponent::Flag;
	CameraData cd;
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);
			//update camera orientation
			cc->Camera.PitchRelative(g_Input.GetMouseDelta().y * TURN_SPEED * -1);
			cc->Camera.YawWorld(g_Input.GetMouseDelta().x * TURN_SPEED * -1);

			//set velocity
			glm::vec3 velocity = glm::vec3(0);
			if (g_Input.IsKeyDown(GLFW_KEY_W)) {
				glm::vec3 f = cc->Camera.GetForward();
				f = glm::normalize(f);
				velocity += f * MOVE_SPEED;
			}
			if (g_Input.IsKeyDown(GLFW_KEY_S)) {
				glm::vec3 f = cc->Camera.GetForward();
				f = glm::normalize(f);
				velocity += f * MOVE_SPEED * -1.0f;
			}
			if (g_Input.IsKeyDown(GLFW_KEY_A)) {
				glm::vec3 f = cc->Camera.GetRight();
				f = glm::normalize(f);
				velocity += f * MOVE_SPEED * -1.0f;
			}
			if (g_Input.IsKeyDown(GLFW_KEY_D)) {
				glm::vec3 f = cc->Camera.GetRight();
				f = glm::normalize(f);
				velocity += f * MOVE_SPEED;
			}
			if (g_Input.IsKeyDown(GLFW_KEY_SPACE)) {
				velocity += glm::vec3(0, MOVE_SPEED, 0);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_C)) {
				velocity += glm::vec3(0, MOVE_SPEED * -1, 0);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
				velocity *= SPEED_DOWN;
			}
			if (g_Input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
				velocity *= SPEED_UP;
			}

			cc->Camera.MoveWorld(velocity * (float)deltaTime);
			cc->Camera.CalculateViewProjection();
		}
	}
#endif
}

void SSCamera::Shutdown() {

}