#include "RenderQueue.h"
#include "BufferManager.h"
#include "ModelBank.h"
#include "MaterialBank.h"

RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::Init(DX12Context* context) {
	m_Context = context;
	m_InstanceCounter = 0;
	g_BufferManager.CreateIndirectBuffer("IndirectBuffer", nullptr, sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT);
	g_BufferManager.CreateIndirectBuffer("IndirectOccluderBuffer", nullptr, sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT);
	g_BufferManager.CreateStructuredBuffer("ShaderInputBuffer", nullptr, MAX_SHADER_INPUT_COUNT * sizeof(ShaderInput), sizeof(ShaderInput));
}

void RenderQueue::UpdateBuffer() {
	g_BufferManager.UpdateBuffer("ShaderInputBuffer", m_ShaderInputBuffer.data(), (unsigned)(m_ShaderInputBuffer.size() * sizeof(ShaderInput)));
	g_BufferManager.UpdateBuffer("IndirectBuffer", m_DrawCalls.data(), (unsigned)(m_DrawCalls.size() * sizeof(IndirectDrawCall)));
	g_BufferManager.UpdateBuffer("IndirectOccluderBuffer", m_OccluderDrawCalls.data(), (unsigned)(m_OccluderDrawCalls.size() * sizeof(IndirectDrawCall)));
}

void RenderQueue::Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs) {
	m_ShaderInputBuffer.insert(m_ShaderInputBuffer.end(), inputs.begin(), inputs.end());
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);
	for (auto& mesh : mod.Meshes) {
		drawCall.DrawIndex = m_InstanceCounter;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset;;

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
	m_ShaderInputBuffer.push_back(input);
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);
	for (auto& mesh : mod.Meshes) {
		drawCall.DrawIndex = m_InstanceCounter;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset * MATERIAL_SIZE;

		drawCall.DrawArgs.InstanceCount = 1;
		drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
		drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
		drawCall.DrawArgs.BaseVertexLocation = 0;
		drawCall.DrawArgs.StartInstanceLocation = 0;

		m_DrawCalls.push_back(drawCall);
	}

	m_InstanceCounter += 1;
}

void RenderQueue::EnqueueOccluder(ModelHandle occluderModel) {
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(occluderModel);
	for (auto& mesh : mod.Meshes) {
		drawCall.DrawIndex = m_InstanceCounter - 1;
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