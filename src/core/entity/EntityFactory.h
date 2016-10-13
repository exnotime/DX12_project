#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <string>
#include <glm/gtc/quaternion.hpp>
#include "../gfx/ShapeGenerator.h"

void SpawnPlayer(const glm::vec3& pos, const glm::vec3& size, const glm::quat& orientation);

void SpawnLevelObject(const std::string modelFilename, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color);
void SpawnLevelObjectM(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color);
void SpawnLevelObjectS(int shape, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color);
void SpawnLevelObjectO(int model, int occluder, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color);

void SpawnOccluder(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale);

void SpawnPhysicsObject(const std::string modelFilename, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass);
void SpawnPhysicsObjectM(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass);
void SpawnPhysicsObjectS(int shape, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass);
void SpawnPhysicsObjectWithForce(int model, const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale, const glm::vec4& color, float mass,const glm::vec3& force);



void RegisterScriptFunctions();