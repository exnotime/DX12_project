#pragma once
#include <assimp/scene.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Model.h"

class Animation {
public:
	Animation();
	~Animation();
	void Load(aiAnimation* animation, Skelleton& bindPose);
	void CalcPose(float time);
private:
	aiAnimation* m_Animation;
	std::vector<Pose> m_KeyFrames;
	Pose m_CurrentPose;
	Pose m_StartPose;
	float m_Duration;
	float m_TicksPerSec;

	struct Transform {
		float Time;
		glm::mat4 Matrix;
	};
};