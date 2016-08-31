#pragma once
#include <assimp/scene.h>
#include "Model.h"

#define g_AnimationBank AnimationBank::GetInstance()
class AnimationBank {
public:
	~AnimationBank();
	static AnimationBank& GetInstance();
	void LoadAnimations(Model& model, aiScene* scene);
private:
	AnimationBank();
};