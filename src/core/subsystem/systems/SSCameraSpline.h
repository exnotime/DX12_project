#pragma once
#include "../SubSystem.h"
#include "../../utilities/CRSpline.h"
class SSCameraSpline : public SubSystem {
public:
	SSCameraSpline();
	~SSCameraSpline();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();

	void AddPointToSpline(glm::vec3 point);
	void SetLookatPos(glm::vec3 pos);
private:
	CRSpline m_Spline;
	glm::vec3 m_LookAtPos;
};

