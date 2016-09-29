#include "PhysicsEngine.h"
#include <Bullet/btBulletCollisionCommon.h>
#include <par_shapes.h>
PhysicsEngine::PhysicsEngine() {

}

PhysicsEngine::~PhysicsEngine() {

}

PhysicsEngine& PhysicsEngine::GetInstance() {
	static PhysicsEngine instance;
	return instance;
}

void PhysicsEngine::Init() {
	m_CollisionConfig = new btDefaultCollisionConfiguration();
	m_Dispatcher = new btCollisionDispatcher(m_CollisionConfig);
	m_PairCache = new btDbvtBroadphase();
	m_Solver = new btSequentialImpulseConstraintSolver;
	m_World = new btDiscreteDynamicsWorld(m_Dispatcher, m_PairCache, m_Solver, m_CollisionConfig);

	m_World->setGravity(btVector3(0, -9.2f, 0));
}

btRigidBody* PhysicsEngine::AddPhysicsObject(float mass, glm::vec3 pos, glm::vec3 size) {
	btBoxShape* object = new btBoxShape(btVector3(size.x,size.y,size.z));
	btDefaultMotionState* state = new btDefaultMotionState();
	btTransform transform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z));
	state->setWorldTransform(transform);

	btVector3 inertia;
	object->calculateLocalInertia(mass, inertia);
	btRigidBody* body = new btRigidBody(mass,state, object, inertia);
	body->setRestitution(0.0f);
	m_World->addRigidBody(body);
	return body;
}

btRigidBody* PhysicsEngine::AddPhysicsObjectS(BASIC_SHAPE shape, float mass, glm::vec3 pos, glm::vec3 size) {
	btCollisionShape* object;
	par_shapes_mesh_s* mesh;
	switch (shape)
	{
	case CUBE:
		object = new btBoxShape(btVector3(size.x, size.y, size.z));
		break;
	case PLANE:
		object = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
		break;
	case SPHERE_SUBDIV:
		object = new btSphereShape(glm::max(glm::max(size.x, size.y), size.z));
		break;
	case SPHERE_PARA:
		object = new btSphereShape(glm::max(glm::max(size.x, size.y), size.z));
		break;
	case CYLINDER:
		object = new btCylinderShape(btVector3(size.x, size.y, size.z));
		break;
	case CAPSULE:
		object = new btCapsuleShape(size.x, size.y); //size.x is radius and size.y is length/height of capsule
		break;
	case DONUT:
		mesh = par_shapes_create_torus(8, 8, 0.5f);
		object = new btConvexHullShape(mesh->points, mesh->npoints, sizeof(float) * 3);
		par_shapes_free_mesh(mesh);
		break;
	case OCTOHEDRON:
		mesh = par_shapes_create_octohedron();
		object = new btConvexHullShape(mesh->points, mesh->npoints, sizeof(float) * 3);
		par_shapes_free_mesh(mesh);
		break;
	case TETRAHEDRON:
		mesh = par_shapes_create_tetrahedron();
		object = new btConvexHullShape(mesh->points, mesh->npoints, sizeof(float) * 3);
		par_shapes_free_mesh(mesh);
		break;
	case DODECAHEDRON:
		mesh = par_shapes_create_dodecahedron();
		object = new btConvexHullShape(mesh->points, mesh->npoints, sizeof(float) * 3);
		par_shapes_free_mesh(mesh);
		break;
	case ICOSAHEDRON:
		mesh = par_shapes_create_icosahedron();
		object = new btConvexHullShape(mesh->points, mesh->npoints, sizeof(float) * 3);
		par_shapes_free_mesh(mesh);
		break;
	default:
		object = new btBoxShape(btVector3(size.x, size.y, size.z));
		printf("unknown shape in the physics engine\n");
		break;
	}
	
	btDefaultMotionState* state = new btDefaultMotionState();
	btTransform transform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z));
	state->setWorldTransform(transform);

	btVector3 inertia;
	object->calculateLocalInertia(mass, inertia);
	btRigidBody* body = new btRigidBody(mass, state, object, inertia);
	body->setRestitution(0.0f);
	m_World->addRigidBody(body);
	return body;
}

void PhysicsEngine::Update(const float deltatime) {
	m_World->stepSimulation(deltatime, 10);
}

void PhysicsEngine::RemovePhysicsObjectFromEntity(unsigned int UID) {
	PhysicsObject* physObj = nullptr;
	int i;
	for (i = 0; i < m_PhysicsObjects.size(); i++) {
		if (m_PhysicsObjects[i]->EntityUID == UID) {
			physObj = m_PhysicsObjects[i];
			break;
		}
	}
	if (!physObj)
		return;
	if (physObj->Body && physObj->Body->getMotionState() && physObj->Body->getCollisionShape()) {
		delete physObj->Body->getMotionState();
		delete physObj->Body->getCollisionShape();
	}
	m_World->removeRigidBody(physObj->Body);
	delete physObj->Body;
	physObj->Body = nullptr;
	physObj->EntityUID = 0;
	delete physObj;
	m_PhysicsObjects.erase(m_PhysicsObjects.begin() + i);
}

void PhysicsEngine::ApplyExplosion(const glm::vec3& position, float radius, float force) {
	btCollisionWorld::ConvexResultCallback* callback;
	btTransform transform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(position.x, position.y, position.z));
	m_World->convexSweepTest(&btSphereShape(radius), transform, transform, *callback);
	if (callback->hasHit()) {
	}
}

void PhysicsEngine::Shutdown() {
	for (int i = m_World->getNumCollisionObjects() - 1; i >= 0; --i) {
		btCollisionObject* obj = m_World->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState() && body->getCollisionShape()) {
			delete body->getMotionState();
			delete body->getCollisionShape();
		}
		m_World->removeCollisionObject(obj);
		delete body;
	}
	for (auto& obj : m_PhysicsObjects) {
		if (obj) delete obj;
	}
	m_PhysicsObjects.clear();

	if (m_World) delete m_World;
	if (m_Solver) delete m_Solver;
	if (m_PairCache) delete m_PairCache;
	if (m_Dispatcher) delete m_Dispatcher;
	if (m_CollisionConfig) delete m_CollisionConfig;
}