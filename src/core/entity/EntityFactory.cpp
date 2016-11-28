#include "EntityFactory.h"
#include "EntityManager.h"
#include "../components/TransformComponent.h"
#include "../components/ModelComponent.h"
#include "../components/CameraComponent.h"
#include "../datasystem/ComponentManager.h"
#include "../script/ScriptEngine.h"
#include <gfx/ModelBank.h>


void SpawnPlayer(const glm::vec3& position, const glm::vec3& size, const glm::quat& orientation) {
	Entity& e = g_EntityManager.CreateEntity();
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = orientation;
	tc.Scale = size;
	tc.World = glm::translate(tc.Position) * glm::mat4_cast(tc.Orientation) * glm::scale(tc.Scale);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	CameraComponent cc;
	cc.Camera.GetEditableData().Fov = (35.0f / 360.0f) * glm::pi<float>() * 2;
	cc.Camera.GetEditableData().Far = 200.0f;
	cc.Camera.GetEditableData().Near = 0.1f;
	cc.Camera.SetOrientation(tc.Orientation);
	cc.Camera.SetPosition(tc.Position);
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
	mc.Occluder = g_ShapeGenerator.GenerateModel((BASIC_SHAPE)shape);
	mc.Color = color;
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);
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
		"void SpawnLevelObject(int model, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObjectM),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnLevelObjectWithOccluder(int model,int occluder, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObjectO),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnShape(int model, vec3 pos, quat orientation, vec3 scale, vec4 color)",
		AngelScript::asFUNCTION(SpawnLevelObjectS),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"int LoadModel(const string file)", AngelScript::asFUNCTION(LoadModel), AngelScript::asCALL_CDECL);

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void SpawnPlayer(vec3 position, vec3 size, quat orientation)", AngelScript::asFUNCTION(SpawnPlayer), AngelScript::asCALL_CDECL);
}