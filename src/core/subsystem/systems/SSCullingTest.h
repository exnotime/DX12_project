#pragma once
#include "../SubSystem.h"
class SSCullingTest : public SubSystem {
public:
	SSCullingTest();
	~SSCullingTest();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:

};

