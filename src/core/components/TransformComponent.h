#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
struct TransformComponent {
	static unsigned int Flag;
	glm::mat4 World;
	glm::vec3 Position;
	glm::vec3 Scale;
	glm::quat Orientation;
};

