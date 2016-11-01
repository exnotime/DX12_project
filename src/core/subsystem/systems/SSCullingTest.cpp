#include "SSCullingTest.h"
#include <core/Input/Input.h>

#include <core/script/ScriptEngine.h>

void AddTest(const std::string name, bool culling, bool async, int duration, int batchSize, int batchCount) {
	TestData td;
	td.Culling = culling;
	td.AsyncCompute = async;
	td.BatchCount = batchCount;
	td.BatchSize = batchSize;
	td.Duration = duration;
	td.TestName = name;
	g_TestParams.Tests.push(td);
}

SSCullingTest::SSCullingTest(){

}

SSCullingTest::~SSCullingTest(){

}

void SSCullingTest::Startup() {
	g_TestParams.UseCulling = true;

	g_ScriptEngine.GetEngine()->RegisterGlobalFunction(
		"void AddTest(string name, bool culling, bool async, int duration, int batchSize, int batchCount)",
		AngelScript::asFUNCTION(AddTest),
		AngelScript::asCALL_CDECL);

	g_ScriptEngine.CompileScript("script/Tests.as");
	g_ScriptEngine.RunScript("script/Tests.as");

	GetNextTest();
}

void SSCullingTest::Update(const double deltaTime) {
	if (g_Input.IsKeyPushed(GLFW_KEY_F9)) {
		g_TestParams.UseCulling = !g_TestParams.UseCulling;
	}
	if (g_Input.IsKeyPushed(GLFW_KEY_F1)) {
		g_TestParams.BatchCount = 128;
		g_TestParams.BatchSize = 128;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F2)) {
		g_TestParams.BatchCount = 256;
		g_TestParams.BatchSize = 256;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F3)) {
		g_TestParams.BatchCount = 512;
		g_TestParams.BatchSize = 512;
		g_TestParams.Reset = true;
	}else if (g_Input.IsKeyPushed(GLFW_KEY_F4)) {
		g_TestParams.BatchCount = 1024;
		g_TestParams.BatchSize = 1024;
		g_TestParams.Reset = true;
	}

#ifdef DO_TESTING
	m_FrameCounter++;
	if (m_FrameCounter > m_CurrentTest.Duration) {
		GetNextTest();
	}
#endif
}

void SSCullingTest::Shutdown() {
}

void SSCullingTest::GetNextTest() {
	if (g_TestParams.Tests.empty()) {
		exit(0); // no more tests either quit or just idle
		//m_CurrentTest.Duration = INT_MAX;
		return;
	}
#ifndef DO_TESTING
	return;
#endif

	m_CurrentTest = g_TestParams.Tests.front();

	g_TestParams.Filename = m_CurrentTest.TestName;
	g_TestParams.UseCulling = m_CurrentTest.Culling;
	g_TestParams.BatchCount = m_CurrentTest.BatchCount;
	g_TestParams.BatchSize = m_CurrentTest.BatchSize;
	g_TestParams.Reset = true;

	m_TestCounter++;
	m_FrameCounter = 0;
	g_TestParams.Tests.pop();
}