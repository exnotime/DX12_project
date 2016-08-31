#pragma once
#include "Window.h"
#include "subsystem/SubSystemSet.h"
namespace core {
	class Engine {
	public:
		Engine();
		~Engine();
		void Init();
		void Run();
	private:
		SubSystemSet m_SubSystemSet;
	};
}