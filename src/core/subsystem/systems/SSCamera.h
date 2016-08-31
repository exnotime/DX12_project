#pragma once
#include "../SubSystem.h"
class SSCamera : public SubSystem {
public:
	SSCamera();
	~SSCamera();

	virtual void Startup();
	virtual void Update(const float deltaTime);
	virtual void Shutdown();
private:

};