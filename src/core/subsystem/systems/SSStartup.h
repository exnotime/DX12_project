#pragma once
#include "../SubSystem.h"
class SSStartup : public SubSystem {
public:
	SSStartup();
	~SSStartup();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:
};

