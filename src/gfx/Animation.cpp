#include "Animation.h"
#include <queue>
#include <algorithm>
#include <glm/gtx/transform.hpp>

Animation::Animation() {

}

Animation::~Animation() {

}

void Animation::Load(aiAnimation* animation, Skelleton& bindPose) {
	m_Duration = animation->mDuration;
	m_TicksPerSec = animation->mTicksPerSecond;
	//each channel represents a bone in the skelleton
	//every key is part of a keyframe
	int boneSize = bindPose.Bones.size();
	m_CurrentPose.Joints.resize(boneSize);
	m_StartPose.Joints.resize(boneSize);
	m_KeyFrames.resize(animation->mChannels[0]->mNumPositionKeys);
	for (auto& key : m_KeyFrames) {
		key.Joints.resize(boneSize);
	}

	for (int c = 0; c < animation->mNumChannels; c++) {
		aiNodeAnim* node = animation->mChannels[c];
		int boneIndex = bindPose.BoneMapping.find(node->mNodeName.data)->second;
		glm::mat4 translate, rotate, scale;

		for (int i = 0; i < node->mNumPositionKeys; i++) {
			m_KeyFrames[i].Time = node->mPositionKeys[i].mTime;
			translate = glm::translate(glm::vec3(node->mPositionKeys[i].mValue.x, node->mPositionKeys[i].mValue.y, node->mPositionKeys[i].mValue.z));
			rotate = glm::mat4_cast( glm::quat(node->mRotationKeys[i].mValue.w, node->mRotationKeys[i].mValue.x, node->mRotationKeys[i].mValue.y, node->mRotationKeys[i].mValue.z));
			scale = glm::scale(glm::vec3(node->mScalingKeys[i].mValue.x, node->mScalingKeys[i].mValue.y, node->mScalingKeys[i].mValue.z));
			m_KeyFrames[i].Joints[boneIndex] = (translate * rotate * scale);
		}
	}
	m_StartPose = m_KeyFrames[0];
	m_CurrentPose = m_StartPose;
}

void Animation::CalcPose(float time) {
	int i;
	float lerp;
	for (i = 0; i < m_KeyFrames.size() - 1; i++) {
		if (m_KeyFrames[i + 1].Time > time) {
			float delta = m_KeyFrames[i + 1].Time - m_KeyFrames[i].Time;
			lerp = (time - m_KeyFrames[i].Time) / delta;
			break;
		}
	}
	m_CurrentPose.Time = time;
	for (int j = 0; j < m_CurrentPose.Joints.size(); j++) {
		m_CurrentPose.Joints[j] = glm::mix(m_KeyFrames[i].Joints[j], m_KeyFrames[i + 1].Joints[j], lerp);
	}
}