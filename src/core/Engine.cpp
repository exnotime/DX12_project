#include "engine.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <sstream>

#include "entity/EntityManager.h"
#include "datasystem/ComponentManager.h"
#include "Input/Input.h"
#include "Timer.h"
#include "Window.h"
#include "subsystem/systems/SSCamera.h"
#include "subsystem/systems/SSGraphics.h"
#include "subsystem/systems/SSPhysics.h"
#include "subsystem/systems/SSScript.h"
#include "entity/EntityFactory.h"
#include "script/ScriptEngine.h"
#include "threading/JobManager.h"
#include "../gfx/ShapeGenerator.h"
#include "../gfx/TestParams.h"

using namespace core;

Engine::Engine() {

}

Engine::~Engine() {
	m_SubSystemSet.ShutdownSubSystems();
	glfwTerminate();
}

void Engine::Init() {
	//g_JobManager.Init(3);
	g_TestParams.UseCulling = true;
	g_ComponentManager.Init();
	g_ScriptEngine.Init();
	RegisterScriptFunctions();

	WindowSettings ws;
	ws.Width = 1600;
	ws.Height = 900;
	ws.Title = "A DX12 Window";
	ws.Vsync = false;
	ws.BorderLess = false;
	ws.Fullscreen = false;
	g_Window.Initialize(ws);

	m_SubSystemSet.AddSubSystem( new SSPhysics() );
	m_SubSystemSet.AddSubSystem( new SSGraphics() );
	m_SubSystemSet.AddSubSystem( new SSCamera() );
	m_SubSystemSet.AddSubSystem( new SSScript() );
	m_SubSystemSet.StartSubSystems();

	g_ShapeGenerator.LoadAllShapes();

	glfwSetKeyCallback(g_Window.GetWindow(), KeyboardCallBack);
	glfwSetMouseButtonCallback(g_Window.GetWindow(), MouseButtonCallback);
	glfwSetCursorPosCallback(g_Window.GetWindow(), MousePosCallback);
	g_Input.SetCursorMode(g_Window.GetWindow(), GLFW_CURSOR_DISABLED);
}

void Engine::Run() {
	int mode = GLFW_CURSOR_DISABLED;
	Timer gameTime;
	while (!glfwWindowShouldClose(g_Window.GetWindow())) {
		if (g_Input.IsKeyPushed(GLFW_KEY_L)) {
			mode = (mode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
			g_Input.SetCursorMode(g_Window.GetWindow(), mode);
		}
		double deltaTime = gameTime.Tick();
		m_SubSystemSet.UpdateSubSystems(deltaTime);

		static int counter = 0;
		static float dt = 0;

		dt += deltaTime;
		counter++;
		if (counter == 100) {
			std::stringstream ss;
			ss << "FPS : " << ceil(1.0 / (dt / counter));
			glfwSetWindowTitle(g_Window.GetWindow(), ss.str().c_str());
			counter = 0;
			dt = 0;
		}
		
		if (g_Input.IsKeyDown(GLFW_KEY_ESCAPE))
			break;

		g_Input.Update();
		glfwPollEvents();
	}
}