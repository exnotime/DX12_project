#pragma once
#include "../SubSystem.h"
#include <string>
#include <gfx/TestParams.h>

class SSCullingTest : public SubSystem {
public:
	SSCullingTest();
	~SSCullingTest();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:
	void GetNextTest();
	int m_TestCounter = 0;
};


