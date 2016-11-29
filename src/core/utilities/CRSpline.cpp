#include "CRSpline.h"

CRSpline::CRSpline() {

}

CRSpline::~CRSpline() {

}

void CRSpline::AddPoint(glm::vec3 point) {
	m_Points.push_back(point);
	if (m_Points.size() > 1) {
		m_DeltaT = 1.0f / (m_Points.size() - 1);
	}
}

glm::vec3 CRSpline::GetPointAtT(float t) {
	if (m_Points.size() < 4)
		return glm::vec3(0);
	t = glm::clamp(t, 0.0f, 1.0f);

	int p = (int)(t / m_DeltaT);
	int size = m_Points.size() - 1;

	int p0 = glm::clamp(p - 1, 0, size);
	int p1 = glm::clamp(p    , 0, size);
	int p2 = glm::clamp(p + 1, 0, size);
	int p3 = glm::clamp(p + 2, 0, size);

	float rt = (t - m_DeltaT* p) / m_DeltaT;
	float rt2 = powf(rt, 2);
	float rt3 = powf(rt, 3);

	glm::vec4 b = glm::vec4((-rt3 + 2 * rt2 - rt) / 2,
		(3 * rt3 - 5 * rt2 + 2) / 2,
		(-rt3 * 3 + 4 * rt2 + rt) / 2,
		(rt3 - rt2) / 2);

	return b[0] * m_Points[p0] + b[1] * m_Points[p1] + b[2] * m_Points[p2] + b[3] * m_Points[p3];

}