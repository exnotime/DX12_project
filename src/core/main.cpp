#include "Engine.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void InterpArgs(int count, char** args) {
	if (count > 0)
		int i = 0;
}

int main(int argc, char** args){
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InterpArgs(argc, args);

	core::Engine engine;
	engine.Init();
	engine.Run();
	return 0;
}