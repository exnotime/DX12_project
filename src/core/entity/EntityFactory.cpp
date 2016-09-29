#include "EntityFactory.h"
#include "EntityManager.h"
#include "components/TransformComponent.h"
#include "components/ModelComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/CameraComponent.h"
#include "datasystem/ComponentManager.h"
#include "../gfx/ModelBank.h"
#include "../physics/PhysicsEngine.h"
#include "../script/ScriptEngine.h"

void SpawnPlayer(const glm::vec3& position, const glm::vec3& size) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = glm::quat(1.0f,0.0f,0.0f,0.0f);
	tc.Scale = size;
	tc.World = glm::translate(tc.Position) * glm::mat4_cast(tc.Orientation) * glm::scale(tc.Scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	CameraComponent cc;
	cc.Camera.GetEditableData().Fov = (60.0f / 360.0f) * glm::pi<float>() * 2;
	cc.Camera.GetEditableData().Far = 1000.0f;
	g_ComponentManager.CreateComponent(&cc, e, CameraComponent::Flag);
}

void SpawnLevelObject(const std::string modelFilename, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = g_ModelBank.LoadModel(modelFilename.c_str());
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);

}

void SpawnLevelObjectM(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = model;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);
}

void SpawnLevelObjectO(int model, int occluder, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = model;
	mc.Occluder = occluder;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);
}

void SpawnLevelObjectS(int shape, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = g_ShapeGenerator.GenerateModel((BASIC_SHAPE)shape);
	mc.Occluder = mc.Model;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);
}

void SpawnPhysicsObject(const std::string modelFilename, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = g_ModelBank.LoadModel(modelFilename.c_str());
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);

	RigidBodyComponent rbc;
	rbc.Body = g_PhysicsEngine.AddPhysicsObject(mass, position, scale);
	g_ComponentManager.CreateComponent(&rbc, e, RigidBodyComponent::Flag);
}

void SpawnPhysicsObjectM(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = model;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);

	RigidBodyComponent rbc;
	rbc.Body = g_PhysicsEngine.AddPhysicsObject(mass, position, scale);
	g_ComponentManager.CreateComponent(&rbc, e, RigidBodyComponent::Flag);
}

void SpawnPhysicsObjectS(int shape, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = g_ShapeGenerator.GenerateModel((BASIC_SHAPE)shape);;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);

	RigidBodyComponent rbc;
	//special special case for special shapes that need special treatment
	glm::vec3 objectscale = scale;
	if (shape == CAPSULE) {
		objectscale = glm::vec3(1, 2, 0);
	}
		

	rbc.Body = g_PhysicsEngine.AddPhysicsObjectS((BASIC_SHAPE)shape, mass, position, objectscale);
	g_ComponentManager.CreateComponent(&rbc, e, RigidBodyComponent::Flag);
}

void SpawnPhysicsObjectWithForce(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass, const glm::vec3& force) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = scale;
	tc.World = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.Model = model;
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);

	RigidBodyComponent rbc;
	rbc.Body = g_PhysicsEngine.AddPhysicsObject(mass, position, scale);
	rbc.Body->applyForce(btVector3(force.x, force.y, force.z), btVector3(0,0,0));
	g_ComponentManager.CreateComponent(&rbc, e, RigidBodyComponent::Flag);
}


static int LoadModel(const std::string file) {
	return g_ModelBank.LoadModel(file.c_str());
}

void RegisterScriptFunctions() {
	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnLevelObject(const string filename, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObject),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPhysicsObject(const string filename, vec3 pos, quat orientation, vec3 scale, vec4 color, float mass)",
		AngelScript::asFUNCTION(SpawnPhysicsObject),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnLevelObject(int model, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObjectM),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnShape(int model, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObjectS),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPhysicsObject(int model, vec3 pos, quat orientation, vec3 scale, vec4 color, float mass)",
		AngelScript::asFUNCTION(SpawnPhysicsObjectM),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPhysicsShape(int shape, vec3 pos, quat orientation, vec3 scale, vec4 color, float mass)",
		AngelScript::asFUNCTION(SpawnPhysicsObjectS),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPhysicsObject(int model, vec3 pos, quat orientation, vec3 scale, vec4 color, float mass, vec3 force)",
		AngelScript::asFUNCTION(SpawnPhysicsObjectWithForce),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"int LoadModel(const string file)", AngelScript::asFUNCTION(LoadModel), AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPlayer(vec3 position, vec3 size)", AngelScript::asFUNCTION(SpawnPlayer), AngelScript::asCALL_CDECL);
}