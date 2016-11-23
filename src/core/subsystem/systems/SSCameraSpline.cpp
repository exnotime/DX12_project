#include "SSCameraSpline.h"
#include "../../script/ScriptEngine.h"
#include <angelscript.h>
using AngelScript::asCALL_THISCALL_ASGLOBAL;
using AngelScript::asSMethodPtr;

#include <gfx/TestParams.h>
#include "../../entity/EntityManager.h"
#include "../../components/CameraComponent.h"
#include "../../datasystem/ComponentManager.h"

SSCameraSpline::SSCameraSpline(){

}

SSCameraSpline::~SSCameraSpline(){

}

void SSCameraSpline::Startup() {
	g_ScriptEngine.GetEngine()->RegisterGlobalFunction("void AddSplinePoint(vec3 point)", asMETHOD(SSCameraSpline, AddPointToSpline), asCALL_THISCALL_ASGLOBAL, this);
}

void SSCameraSpline::Update(const double deltaTime) {
	if (!g_TestParams.FreeCamera) {
		for (auto& entity : g_EntityManager.GetEntityList()) {
			if ((entity.ComponentBitfield & CameraComponent::Flag) == CameraComponent::Flag) {
				CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(entity, CameraComponent::Flag);

				if (g_TestParams.Reset) {
					cc->Camera = Camera(); // reset camera
				}
				float t = g_TestParams.FrameCounter / (float)g_TestParams.CurrentTest.Duration;
				glm::vec3 pos = m_Spline.GetPointAtT(t);
				cc->Camera.LookAt(pos);
				cc->Camera.SetPosition(pos);
				cc->Camera.CalculateViewProjection();


			}
		}
	}
}

void SSCameraSpline::Shutdown() {
}

void SSCameraSpline::AddPointToSpline(glm::vec3 point) {
	m_Spline.AddPoint(point);
}

