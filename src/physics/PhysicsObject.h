#pragma once
#include <Bullet/btBulletDynamicsCommon.h>

struct PhysicsObject {
	btRigidBody* Body;
	unsigned int EntityUID;
};