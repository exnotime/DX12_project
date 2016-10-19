#include "SSCullingTest.h"
#include <Input/Input.h>
#include "../gfx/TestParams.h"
SSCullingTest::SSCullingTest(){

}

SSCullingTest::~SSCullingTest(){

}

void SSCullingTest::Startup() {
	g_TestParams.UseCulling = true;
}

void SSCullingTest::Update(const double deltaTime) {
	if (g_Input.IsKeyPushed(GLFW_KEY_F9)) {
		g_TestParams.UseCulling = !g_TestParams.UseCulling;
	}
}

void SSCullingTest::Shutdown() {
}

