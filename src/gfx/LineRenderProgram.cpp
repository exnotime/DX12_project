#include "LineRenderProgram.h"
#include "RootSignatureFactory.h"
#include "PipelineStateFactory.h"
LineRenderProgram::LineRenderProgram() {

}

LineRenderProgram::~LineRenderProgram() {

}

void LineRenderProgram::Init(DX12Context* context) {
	m_Shader.LoadFromFile(L"src/shaders/Line.hlsl", VERTEX_SHADER_BIT | PIXEL_SHADER_BIT);

	RootSignatureFactory rootFact;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
	CD3DX12_DESCRIPTOR_RANGE r;
	r.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges.push_back(r);
	rootFact.AddDescriptorTable(ranges);
	rootFact.AddConstant(1, 1); //index
	m_RootSignature = rootFact.CreateSignture(context->Device.Get());

	const D3D12_INPUT_ELEMENT_DESC lineLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	PipelineStateFactory pipeFact;
	pipeFact.SetAllShaders(m_Shader);
	pipeFact.SetInputLayout(lineLayout, 1);
	pipeFact.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
	pipeFact.AddRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipeFact.SetDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	CD3DX12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterDesc.AntialiasedLineEnable = true;
	pipeFact.SetRasterizerState(rasterDesc);

	pipeFact.SetRootSignature(m_RootSignature.Get());
	m_PipelineState = pipeFact.CreateGraphicsState(context);

	UINT realSize = ((sizeof(glm::mat4) + sizeof(glm::vec4) * 16) + 255) & ~255;

	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(realSize);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_BufferResource));

	CD3DX12_RESOURCE_DESC vboDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(glm::mat3) * 32 * 1024);
	context->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&vboDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBufferResource));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	context->Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_BufferResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = bufferDesc.Width;
	context->Device->CreateConstantBufferView(&cbvDesc, m_DescHeap->GetCPUDescriptorHandleForHeapStart());

	m_VBO.BufferLocation = m_VertexBufferResource->GetGPUVirtualAddress();
	m_VBO.SizeInBytes = vboDesc.Width;
	m_VBO.StrideInBytes = sizeof(glm::vec3);
}

void LineRenderProgram::Render(ID3D12GraphicsCommandList* cmdList, RenderQueue* queue) {
	//transfer vertices
	D3D12_RANGE range = { 0,0 };
	unsigned char* data;
	m_VertexBufferResource->Map(0, &range, (void**)&data);
	const size_t size = sizeof(glm::vec3) * queue->GetLinePoints().size();
	memcpy(data, queue->GetLinePoints().data(), size);
	range = { 0, size };
	m_VertexBufferResource->Unmap(0, &range);
	//transfer frame data
	std::vector<Line>& lines = queue->GetLines();
	range = { 0,0 };
	m_BufferResource->Map(0, &range, (void**)&data);
	memcpy(data, &(queue->GetViews().at(0).Camera.ProjView), sizeof(glm::mat4)); //transfer camera matrix
	data += sizeof(glm::mat4);
	for (int i = 0; i < lines.size(); ++i) {
		memcpy(data, &lines[i].Color, sizeof(glm::vec4)); //line colors
		data += sizeof(glm::vec4);
	}
	m_BufferResource->Unmap(0, nullptr);
	cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	ID3D12DescriptorHeap* descheaps[] = { m_DescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, descheaps);
	cmdList->SetGraphicsRootDescriptorTable(0, m_DescHeap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetPipelineState(m_PipelineState.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);//linestrip to make continous lines
	cmdList->IASetVertexBuffers(0, 1, &m_VBO);
	for (int i = 0; i < lines.size(); ++i) {
		cmdList->SetGraphicsRoot32BitConstant(1, i, 0);
		cmdList->DrawInstanced(lines[i].End - lines[i].Start, 1, lines[i].Start, 0);
	}
}