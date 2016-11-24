#include "SSCullingTest.h"
#include <core/Input/Input.h>
#include <core/script/ScriptEngine.h>

void AddTest(std::string name, bool culling, int duration,int triangleCount, int batchSize, int batchCount, glm::vec2 framebuffersize, bool instrument, glm::bvec4 filters) {
	TestData td;
	td.Culling = culling;
	td.BatchCount = batchCount;
	td.BatchSize = batchSize;
	td.Duration = duration;
	td.TestName = name;
	td.TriangleCount = triangleCount;
	td.FrameBufferSize = framebuffersize;
	td.Instrument = instrument;
	td.FilterBackFace = filters[0];
	td.FilterSmallTri = filters[1];
	td.FilterFrustum = filters[2];
	td.FilterOcclusion = filters[3];
	g_TestParams.Tests.push(td);
}

SSCullingTest::SSCullingTest(){

}

SSCullingTest::~SSCullingTest(){

}

void SSCullingTest::Startup() {
	g_TestParams.CurrentTest.Culling = true;

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void AddTest(string name, bool culling, int duration, int triangleCount, int batchSize, int batchCount, vec2 framebuffersize, bool instrument, bvec4 filters)",
		AngelScript::asFUNCTION(AddTest),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.CompileScript("script/Tests.as");
	g_ScriptEngine.RunScript("script/Tests.as");

	GetNextTest();
}

void SSCullingTest::Update(const double deltaTime) {
	if (g_Input.IsKeyPushed(GLFW_KEY_F9)) {
		g_TestParams.CurrentTest.Culling = !g_TestParams.CurrentTest.Culling;
	}
	if (g_Input.IsKeyPushed(GLFW_KEY_F1)) {
		g_TestParams.CurrentTest.BatchCount = 128;
		g_TestParams.CurrentTest.BatchSize = 128;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F2)) {
		g_TestParams.CurrentTest.BatchCount = 256;
		g_TestParams.CurrentTest.BatchSize = 256;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F3)) {
		g_TestParams.CurrentTest.BatchCount = 512;
		g_TestParams.CurrentTest.BatchSize = 512;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F4)) {
		g_TestParams.CurrentTest.BatchCount = 1024;
		g_TestParams.CurrentTest.BatchSize = 1024;
		g_TestParams.Reset = true;
	}
	if (g_Input.IsKeyPushed(GLFW_KEY_F5)) {
		g_TestParams.CurrentTest.FilterBackFace = !g_TestParams.CurrentTest.FilterBackFace;
		g_TestParams.Reset = true;
	}
	else if (g_Input.IsKeyPushed(GLFW_KEY_F6)) {
		g_TestParams.CurrentTest.FilterSmallTri = !g_TestParams.CurrentTest.FilterSmallTri;
		g_TestParams.Reset = true;
	}
	else if (g_Input.IsKeyPushed(GLFW_KEY_F7)) {
		g_TestParams.CurrentTest.FilterFrustum = !g_TestParams.CurrentTest.FilterFrustum;
		g_TestParams.Reset = true;
	}
	else if (g_Input.IsKeyPushed(GLFW_KEY_F8)) {
		g_TestParams.CurrentTest.FilterOcclusion = !g_TestParams.CurrentTest.FilterOcclusion;
		g_TestParams.Reset = true;
	}
	
#ifdef DO_TESTING
	g_TestParams.FrameCounter++;
	if (g_TestParams.FrameCounter > g_TestParams.CurrentTest.Duration) {
		GetNextTest();
	}
#endif
}

void SSCullingTest::Shutdown() {
}

void SSCullingTest::GetNextTest() {
#ifndef DO_TESTING
	return;
#endif

	if (g_TestParams.Tests.empty()) {
		//quiting like this will leak a bunch of memory, but since it is a lot easier to just let mommy os clean up after us we will allow it
		exit(0); // no more tests either quit or just idle
		//m_CurrentTest.Duration = INT_MAX;
		return;
	}

	g_TestParams.CurrentTest = g_TestParams.Tests.front();
	g_TestParams.Directory = g_TestParams.CurrentTest.TestName;

	g_TestParams.Reset = true;

	m_TestCounter++;
	g_TestParams.FrameCounter = 0;
	g_TestParams.Tests.pop();
}