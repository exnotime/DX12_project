#include "SSPhysics.h"
#include "../../../physics/PhysicsEngine.h"
#include "../../Input/Input.h"
#include "../../entity/EntityManager.h"
#include "../../datasystem/ComponentManager.h"
#include "../../components/TransformComponent.h"
#include "../../components/RigidBodyComponent.h"
SSPhysics::SSPhysics() {

}

SSPhysics::~SSPhysics() {

}

void SSPhysics::Startup() {
	g_PhysicsEngine.Init();
}

void SSPhysics::Update(const float deltaTime) {
	g_PhysicsEngine.Update(deltaTime);
	//get translation from the bullet rigid body

	int flag = TransformComponent::Flag | RigidBodyComponent::Flag;

	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & flag) == flag) {
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(entity,TransformComponent::Flag);
			RigidBodyComponent* rbc = (RigidBodyComponent*)g_ComponentManager.GetComponent(entity, RigidBodyComponent::Flag);

			if (rbc->Body->isActive()) {
				btTransform transform;
				rbc->Body->getMotionState()->getWorldTransform(transform);
				transform.getOpenGLMatrix(&tc->World[0][0]);

				tc->Orientation = glm::quat_cast(tc->World);
				tc->Position = glm::vec3(tc->World[3]);
			}
		}
	}
}

void SSPhysics::Shutdown() {
	g_PhysicsEngine.Shutdown();
}
