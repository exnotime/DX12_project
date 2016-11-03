#pragma once
#include <glm/glm.hpp>
#include <vector>
class CRSpline {
public:
	CRSpline();
	~CRSpline();
	void AddPoint(const glm::vec3& point);
	glm::vec3 GetPointAtT(float t);
private:
	std::vector<glm::vec3> m_Points;
	float m_DeltaT;
};
