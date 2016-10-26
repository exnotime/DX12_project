#pragma once
#include "../SubSystem.h"
#include <string>
#include <gfx/TestParams.h>

#define DO_TESTING

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
	int m_FrameCounter = 0;
	TestData m_CurrentTest;
};


