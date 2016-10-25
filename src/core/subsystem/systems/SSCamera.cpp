#include "SSCamera.h"
#include "../../datasystem/ComponentManager.h"
#include "../../entity/EntityManager.h"
#include "../../components/CameraComponent.h"
#include "../../Input/Input.h"
#include "../../entity/EntityFactory.h"
#define MOVE_SPEED 10.0f
#define TURN_SPEED 0.004f
SSCamera::SSCamera() {

}

SSCamera::~SSCamera() {

}

void SSCamera::Startup() {
	
}

void SSCamera::Update(const double deltaTime) {
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
				cc->Camera.MoveWorld(glm::vec3(0, MOVE_SPEED * deltaTime, 0));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_C)) {
				cc->Camera.MoveWorld(glm::vec3(0, MOVE_SPEED * deltaTime * -1, 0));
			}
			cc->Camera.MoveWorld(velocity * (float)deltaTime);
			cc->Camera.CalculateViewProjection();
		}
	}
}

void SSCamera::Shutdown() {

}