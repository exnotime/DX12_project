#include "SSCullingTest.h"
#include <core/Input/Input.h>

#include <core/script/ScriptEngine.h>

void AddTest(std::string name, bool culling, bool async, int duration, int batchSize, int batchCount) {
	TestData td;
	td.Culling = culling;
	td.AsyncCompute = async;
	td.BatchCount = batchCount;
	td.BatchSize = batchSize;
	td.Duration = duration;
	td.TestName = name;
	g_TestParams.Tests.push_back(td);
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

#ifdef DO_TESTING
	m_FrameCounter++;
	if (m_CurrentTest.Duration <= m_FrameCounter) {
		GetNextTest();
	}
#endif
}

void SSCullingTest::Shutdown() {
}

void SSCullingTest::GetNextTest() {
	if (m_TestCounter >= g_TestParams.Tests.size()) {
		exit(0); // no more tests either quit or just idle
		//m_CurrentTest.Duration = INT_MAX;
		//return;
	}

	m_CurrentTest = g_TestParams.Tests[m_TestCounter];

	g_TestParams.Filename = m_CurrentTest.TestName;
	g_TestParams.UseCulling = m_CurrentTest.Culling;

	g_TestParams.Reset = true;
	m_TestCounter++;
	m_FrameCounter = 0;
}