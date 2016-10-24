#include "RenderQueue.h"
#include "BufferManager.h"
#include "ModelBank.h"
#include "MaterialBank.h"
#include <glm/gtc/matrix_access.hpp>
RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::Init() {
	m_InstanceCounter = 0;
	g_BufferManager.CreateIndirectBuffer("IndirectBuffer", sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT);
	g_BufferManager.CreateIndirectBuffer("IndirectOccluderBuffer", sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT);
	g_BufferManager.CreateStructuredBuffer("ShaderInputBuffer", MAX_SHADER_INPUT_COUNT * sizeof(ShaderInput), sizeof(ShaderInput));
}

void RenderQueue::UpdateBuffer(ID3D12GraphicsCommandList* cmdList) {
	g_BufferManager.UpdateBuffer(cmdList,"ShaderInputBuffer", m_ShaderInputBuffer.data(), (unsigned)(m_ShaderInputBuffer.size() * sizeof(ShaderInput)));
	g_BufferManager.UpdateBuffer(cmdList,"IndirectBuffer", m_DrawCalls.data(), (unsigned)(m_DrawCalls.size() * sizeof(IndirectDrawCall)));
	g_BufferManager.UpdateBuffer(cmdList,"IndirectOccluderBuffer", m_OccluderDrawCalls.data(), (unsigned)(m_OccluderDrawCalls.size() * sizeof(IndirectDrawCall)));
}

void RenderQueue::Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs) {
	m_ShaderInputBuffer.insert(m_ShaderInputBuffer.end(), inputs.begin(), inputs.end());
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);
	for (auto& mesh : mod.Meshes) {
		drawCall.DrawIndex = m_InstanceCounter;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset;

		drawCall.DrawArgs.InstanceCount = (unsigned)inputs.size();
		drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
		drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
		drawCall.DrawArgs.BaseVertexLocation = 0;
		drawCall.DrawArgs.StartInstanceLocation = 0;

		m_DrawCalls.push_back(drawCall);
	}

	m_InstanceCounter += (unsigned)inputs.size();
}

void RenderQueue::Enqueue(ModelHandle model, const ShaderInput& input) {
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);

	for (auto& mesh : mod.Meshes) {
		glm::vec4 max = input.World * glm::vec4(mesh.Max + mesh.Offset, 1.0f);
		glm::vec4 min = input.World * glm::vec4(mesh.Min + mesh.Offset, 1.0f);
		bool boxfrustum = AABBvsFrustum(max, min);

		//glm::vec4 minToMax = (max - min);
		//float rad = glm::length(minToMax) * 0.5f;
		//glm::vec4 center = (min + minToMax * 0.5f);// +glm::vec4(m_Views[1].Camera.Position, 0);
		//center = m_Views[1].Camera.View * center;
		//bool spherefrustum = SpherevsFrustum(center,rad);

		if (boxfrustum) {
			drawCall.DrawIndex = m_ShaderInputBuffer.size();
			drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset * MATERIAL_SIZE;

			drawCall.DrawArgs.InstanceCount = 1;
			drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
			drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
			drawCall.DrawArgs.BaseVertexLocation = 0;
			drawCall.DrawArgs.StartInstanceLocation = 0;

			m_DrawCalls.push_back(drawCall);
		}
	}
	m_ShaderInputBuffer.push_back(input);
}

void RenderQueue::EnqueueOccluder(ModelHandle occluderModel) {
	//occluders always comes directly after the other model
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(occluderModel);
	for (auto& mesh : mod.Meshes) {

		glm::vec4 max = m_ShaderInputBuffer.back().World * glm::vec4(mesh.Max + mesh.Offset, 1.0f);
		glm::vec4 min = m_ShaderInputBuffer.back().World * glm::vec4(mesh.Min + mesh.Offset, 1.0f);

		if (!AABBvsFrustum(max, min))
			continue;

		drawCall.DrawIndex = m_ShaderInputBuffer.size() - 1;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset * MATERIAL_SIZE;

		drawCall.DrawArgs.InstanceCount = 1;
		drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
		drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
		drawCall.DrawArgs.BaseVertexLocation = 0;
		drawCall.DrawArgs.StartInstanceLocation = 0;

		m_OccluderDrawCalls.push_back(drawCall);
	}
}

void RenderQueue::Clear() {
	m_DrawCalls.clear();
	m_OccluderDrawCalls.clear();

	m_ShaderInputBuffer.clear();
	m_Views.clear();
	m_InstanceCounter = 0;
}

void RenderQueue::AddView(const View& v) {
	m_Views.push_back(v);

	glm::mat4 vp = v.Camera.ProjView;
	glm::vec4 row1 = glm::row(vp, 0);
	glm::vec4 row2 = glm::row(vp, 1);
	glm::vec4 row3 = glm::row(vp, 2);
	glm::vec4 row4 = glm::row(vp, 3);
	m_FrustumPlanes[0] = row4 + row1;
	m_FrustumPlanes[1] = row4 - row1;
	m_FrustumPlanes[2] = row4 + row2;
	m_FrustumPlanes[3] = row4 - row2;
	m_FrustumPlanes[4] = row4 + row3;
	m_FrustumPlanes[5] = row4 - row3;
	for (int i = 0; i < 6; ++i) {
		m_FrustumPlanes[i] = glm::normalize(m_FrustumPlanes[i]);
	}
	glm::mat4 invProj = glm::inverse(v.Camera.ProjView);
	m_FrustumCorners[0] = invProj * glm::vec4(1, 1, 1, 1); m_FrustumCorners[0] /= m_FrustumCorners[0].w;
	m_FrustumCorners[1] = invProj * glm::vec4(-1,  1,  1, 1); m_FrustumCorners[1] /= m_FrustumCorners[1].w;
	m_FrustumCorners[2] = invProj * glm::vec4( 1, -1,  1, 1); m_FrustumCorners[2] /= m_FrustumCorners[2].w;
	m_FrustumCorners[3] = invProj * glm::vec4( 1,  1, -1, 1); m_FrustumCorners[3] /= m_FrustumCorners[3].w;
	m_FrustumCorners[4] = invProj * glm::vec4(-1, -1,  1, 1); m_FrustumCorners[4] /= m_FrustumCorners[4].w;
	m_FrustumCorners[5] = invProj * glm::vec4( 1, -1, -1, 1); m_FrustumCorners[5] /= m_FrustumCorners[5].w;
	m_FrustumCorners[6] = invProj * glm::vec4(-1,  1, -1, 1); m_FrustumCorners[6] /= m_FrustumCorners[6].w;
	m_FrustumCorners[7] = invProj * glm::vec4(-1, -1, -1, 1); m_FrustumCorners[7] /= m_FrustumCorners[7].w;

}

bool RenderQueue::AABBvsFrustum(const glm::vec4& max, const glm::vec4& min) {

	for (int i = 0; i < 6; ++i) {
		int out = 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(min.x, min.y, min.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(max.x, min.y, min.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(min.x, max.y, min.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(max.x, max.y, min.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(min.x, min.y, max.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(max.x, min.y, max.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(min.x, max.y, max.z, 1.0f)) < 0.0f ? 1 : 0;
		out += glm::dot(m_FrustumPlanes[i], glm::vec4(max.x, max.y, max.z, 1.0f)) < 0.0f ? 1 : 0;
		if (out == 8) 
			return false;
	}

	//int out;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].x > max.x) ? 1 : 0); if (out == 8) return false;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].x < min.x) ? 1 : 0); if (out == 8) return false;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].y > max.y) ? 1 : 0); if (out == 8) return false;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].y < min.y) ? 1 : 0); if (out == 8) return false;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].z > max.z) ? 1 : 0); if (out == 8) return false;
	//out = 0; for (int i = 0; i<8; i++) out += ((m_FrustumCorners[i].z < min.z) ? 1 : 0); if (out == 8) return false;

	return true;
}

bool RenderQueue::SpherevsFrustum(const glm::vec4& pos, const float radius) {
	for(int i = 0; i < 6; ++i){
		float d = glm::dot(pos, m_FrustumPlanes[i]);
		if (d < -radius)
   			return false;
	}
	return true;
}
