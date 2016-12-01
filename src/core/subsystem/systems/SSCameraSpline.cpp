#include "SSCameraSpline.h"
#include "../../script/ScriptEngine.h"
#include <angelscript.h>
#include <gfx/TestParams.h>
#include "../../entity/EntityManager.h"
#include "../../components/CameraComponent.h"
#include "../../datasystem/ComponentManager.h"

using AngelScript::asCALL_THISCALL_ASGLOBAL;
using AngelScript::asSMethodPtr;
#define NaNBREAD 0x7fc00000 //angelscript nan(bread)


SSCameraSpline::SSCameraSpline(){
	m_LookAtPos = glm::vec3(NaNBREAD);
}

SSCameraSpline::~SSCameraSpline(){

}

void SSCameraSpline::Startup() {
	g_ScriptEngine.GetEngine()->RegisterGlobalFunction("void AddSplinePoint(vec3 point)", asMETHOD(CRSpline, CRSpline::AddPoint), asCALL_THISCALL_ASGLOBAL, &m_Spline);
	g_ScriptEngine.GetEngine()->RegisterGlobalFunction("void SetLookatPos(vec3 point)", asMETHOD(SSCameraSpline, SSCameraSpline::SetLookatPos), asCALL_THISCALL_ASGLOBAL, this);
}

void SSCameraSpline::Update(const double deltaTime) {
#ifndef FREE_CAMERA
	for (auto& entity : g_EntityManager.GetEntityList()) {
		if ((entity.ComponentBitfield & CameraComponent::Flag) == CameraComponent::Flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);

			if (g_TestParams.Reset) {
				cc->Camera = Camera(); // reset camera
			}
			float t = g_TestParams.FrameCounter / (float)g_TestParams.CurrentTest.Duration;
			glm::vec3 pos = m_Spline.GetPointAtT(t);
			if (m_LookAtPos == glm::vec3(NaNBREAD)) {
				cc->Camera.LookAt(pos);
				cc->Camera.SetPosition(pos);
			} else {
				cc->Camera.SetPosition(pos);
				cc->Camera.LookAt(m_LookAtPos);
			}
			cc->Camera.CalculateViewProjection();
		}
	}
#endif
}

void SSCameraSpline::Shutdown() {
}

void SSCameraSpline::AddPointToSpline(glm::vec3 point) {
	m_Spline.AddPoint(point);
}

void SSCameraSpline::SetLookatPos(glm::vec3 pos) {
	m_LookAtPos = pos;
	printf("Lookat pos: %f, %f, %f\n", pos.x, pos.y, pos.z);
}

