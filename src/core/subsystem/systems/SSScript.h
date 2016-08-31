#pragma once
#include "../SubSystem.h"
class SSScript : public SubSystem {
public:
	SSScript();
	~SSScript();

	virtual void Startup();
	virtual void Update(const float deltaTime);
	virtual void Shutdown();
private:

};

