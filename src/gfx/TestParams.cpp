#include "TestParams.h"
TestParams& TestParams::GetInstance() {
	static TestParams instance;
	return instance;
 }

