#pragma once
#include "../SubSystem.h"

#include "../../../gfx/GraphicsEngine.h"

int JobTest(void* args);

class SSGraphics : public SubSystem {
public:
	SSGraphics();
	~SSGraphics();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:
	GraphicsEngine* m_Graphics;
	RenderQueue* m_RenderQueue;
};

