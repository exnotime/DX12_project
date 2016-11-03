#include "SSScript.h"
#include "../../script/ScriptEngine.h"
#include "../../input/Input.h"
#include "../../entity/EntityManager.h"
SSScript::SSScript(){

}

SSScript::~SSScript(){

}

void SSScript::Startup() {
	printf("Loading all scripts\n");
	g_ScriptEngine.CompileScript("script/spawnLevel.as");
	g_ScriptEngine.RunScript("script/spawnLevel.as");
}

void SSScript::Update(const double deltaTime) {
	if (g_Input.IsKeyPushed(GLFW_KEY_KP_7)) {
		printf("Recompiling all scripts\n");
		g_ScriptEngine.RecompileAllScripts();
	}

	if (g_Input.IsKeyPushed(GLFW_KEY_KP_9)) {
		g_EntityManager.RemoveAllEntities();
		g_ScriptEngine.RunScript("script/spawnLevel.as");
	}
}

void SSScript::Shutdown() {

}

