#pragma once
#include "../SubSystem.h"

#include "../../../gfx/GraphicsEngine.h"

class SSGraphics : public SubSystem {
public:
	SSGraphics();
	~SSGraphics();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:
	bool FrustumCheck(const glm::vec4* planes, const glm::vec3& pos, float radius);
	GraphicsEngine* m_Graphics;
	RenderQueue* m_RenderQueue;
};

