#pragma once
#include <Bullet/btBulletDynamicsCommon.h>
#include "PhysicsObject.h"
#include <vector>
#include <glm/glm.hpp>
#define g_PhysicsEngine PhysicsEngine::GetInstance()
class PhysicsEngine {
public:
	~PhysicsEngine();
	 static PhysicsEngine& GetInstance();
	 void Init();
	 btRigidBody* AddPhysicsObject(float mass, glm::vec3 pos, glm::vec3 size);
	 void RemovePhysicsObjectFromEntity(unsigned int UID);
	 void Update(const float deltatime);
	 void ApplyExplosion(const glm::vec3& position, float radius, float force);
	 void Shutdown();
private:
	PhysicsEngine();

	std::vector<PhysicsObject*> m_PhysicsObjects;

	btDefaultCollisionConfiguration* m_CollisionConfig = nullptr;
	btCollisionDispatcher* m_Dispatcher = nullptr;
	btBroadphaseInterface* m_PairCache = nullptr;
	btSequentialImpulseConstraintSolver* m_Solver = nullptr;
	btDiscreteDynamicsWorld* m_World = nullptr;
};