#include "RenderQueue.h"
#include "BufferManager.h"
#include "ModelBank.h"
#include "MaterialBank.h"

RenderQueue::RenderQueue() {
	m_ModelQueue.reserve(50);
}

RenderQueue::~RenderQueue() {
	m_ArgumentBuffer->Unmap(0, nullptr);
}

void RenderQueue::Init(DX12Context* context) {
	m_Context = context;
	//create indirect args buffer
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_ArgumentBuffer));

	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndirectDrawCall) * MAX_SHADER_INPUT_COUNT), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ArgumentUpload));
	//map buffer
	D3D12_RANGE range = { 0,0 };
	m_ArgumentUpload->Map(0, &range, (void**)&m_IndirectStart);
	m_IndirectCounter = 0;
	m_InstanceCounter = 0;
}

void RenderQueue::CreateBuffer() {
	g_BufferManager.CreateStructuredBuffer("ShaderInputBuffer", nullptr, MAX_SHADER_INPUT_COUNT * sizeof(ShaderInput), sizeof(ShaderInput));
}

void RenderQueue::UpdateBuffer() {
	g_BufferManager.UpdateBuffer("ShaderInputBuffer", m_ShaderInputBuffer.data(), (unsigned)(m_ShaderInputBuffer.size() * sizeof(ShaderInput)));

	D3D12_RANGE range = { 0, m_IndirectCounter * sizeof(IndirectDrawCall) };
	m_ArgumentUpload->Unmap(0, &range);

	m_Context->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ArgumentBuffer.Get(),
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST));

	m_Context->CommandList->CopyBufferRegion(m_ArgumentBuffer.Get(), 0, m_ArgumentUpload.Get(), 0, m_IndirectCounter * sizeof(IndirectDrawCall));

	UINT size = sizeof(IndirectDrawCall);
	m_Context->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ArgumentBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));

	range = { 0,0 };
	m_ArgumentUpload->Map(0, &range, (void**)&m_IndirectStart);
}

void RenderQueue::Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs) {
	//ModelObject mo;
	//mo.Model = model;
	//mo.InstanceCount = (int)inputs.size();
	//m_ModelQueue.push_back(mo);
	m_ShaderInputBuffer.insert(m_ShaderInputBuffer.end(), inputs.begin(), inputs.end());
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);
	for (auto& mesh : mod.Meshes) {
		drawCall.VBO[0] = mesh.VBOView.PosView;
		drawCall.VBO[1] = mesh.VBOView.NormalView;
		drawCall.VBO[2] = mesh.VBOView.TangentView;
		drawCall.VBO[3] = mesh.VBOView.TexView;

		drawCall.DrawIndex = m_InstanceCounter;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset;;

		drawCall.DrawArgs.InstanceCount = (unsigned)inputs.size();
		drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
		drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
		drawCall.DrawArgs.BaseVertexLocation = 0;
		drawCall.DrawArgs.StartInstanceLocation = 0;

		m_IndirectStart[m_IndirectCounter] = drawCall;
		m_IndirectCounter++;
	}

	m_InstanceCounter += (unsigned)inputs.size();


}

void RenderQueue::Enqueue(ModelHandle model, const ShaderInput& input) {
	//ModelObject mo;
	//mo.Model = model;
	//mo.InstanceCount = 1;
	//m_ModelQueue.push_back(mo);
	m_ShaderInputBuffer.push_back(input);
	IndirectDrawCall drawCall;
	Model mod = g_ModelBank.FetchModel(model);
	for (auto& mesh : mod.Meshes) {
		drawCall.VBO[0] = mesh.VBOView.PosView;
		drawCall.VBO[1] = mesh.VBOView.NormalView;
		drawCall.VBO[2] = mesh.VBOView.TangentView;
		drawCall.VBO[3] = mesh.VBOView.TexView;

		drawCall.DrawIndex = m_InstanceCounter;
		drawCall.MaterialOffset = g_MaterialBank.GetMaterial(mod.MaterialHandle + mesh.MaterialOffset)->Offset * MATERIAL_SIZE;

		drawCall.DrawArgs.InstanceCount = 1;
		drawCall.DrawArgs.IndexCountPerInstance = mesh.IndexCount;
		drawCall.DrawArgs.StartIndexLocation = mod.IndexHandle + mesh.IndexBufferOffset;
		drawCall.DrawArgs.BaseVertexLocation = 0;
		drawCall.DrawArgs.StartInstanceLocation = 0;

		m_IndirectStart[m_IndirectCounter] = drawCall;
		m_IndirectCounter++;
	}

	m_InstanceCounter += 1;
}

void RenderQueue::Enqueue(ModelHandle model, const std::vector<ShaderInput>& inputs, float transparency) {
	TransparentModelObject mo;
	mo.Model = model;
	mo.InstanceCount = (int)inputs.size();
	mo.Transparency = transparency;
	m_TransparentModelQueue.push_back(mo);
	m_TransparentShaderInputBuffer.insert(m_TransparentShaderInputBuffer.end(), inputs.begin(), inputs.end());
}

void RenderQueue::Enqueue(ModelHandle model, const ShaderInput& input, float transparency) {
	TransparentModelObject mo;
	mo.Model = model;
	mo.InstanceCount = 1;
	mo.Transparency = transparency;
	m_TransparentModelQueue.push_back(mo);
	m_TransparentShaderInputBuffer.insert(m_TransparentShaderInputBuffer.end(), input);
}

void RenderQueue::Clear() {
	m_ModelQueue.clear();
	m_ShaderInputBuffer.clear();
	m_TransparentShaderInputBuffer.clear();
	m_Views.clear();
	m_TransparentModelQueue.clear();
	m_IndirectCounter = 0;
	m_InstanceCounter = 0;
}