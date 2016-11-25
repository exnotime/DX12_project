#include "Engine.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <gfx/TestParams.h>

void ProcessCmdLineArgs(int argc, char** argv) {
	if (argc < 2) {
		printf("No level name argument set, defaulting to Sponza\n");
		g_TestParams.Level = "SPONZA";
		return;
	}
	g_TestParams.Level = std::string(argv[1]);
}

int main(int argc, char** argv){
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	ProcessCmdLineArgs(argc, argv);
	core::Engine engine;
	engine.Init();
	engine.Run();
	return 0;
}