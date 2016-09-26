#pragma once
#include <glm/glm.hpp>
struct ModelComponent {
	static unsigned int Flag;
	int Model = -1;
	int Occluder = -1;
	glm::vec4 Color;
};

