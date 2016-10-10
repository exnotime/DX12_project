#include "extensions/Extensions.hlsl"

struct DrawCallArgs {
	uint DrawIndex;
	uint MaterialIndex;
	uint IndexCount;
	uint InstanceCount;
	uint IndexOffset;
	uint BaseVertex;
	uint BaseInstance;
	uint Padding;
};

struct ShaderInput {
	float4x4 World;
	float4 Color;
};

StructuredBuffer<DrawCallArgs> g_DrawArgsBuffer : register(t0);
StructuredBuffer<float3> g_VertexPositions : register(t1);
StructuredBuffer<uint> g_TriangleIndices : register(t2);
StructuredBuffer<ShaderInput> g_InputBuffer : register(t3);

RWStructuredBuffer<DrawCallArgs> g_OutDrawArgs : register(u0);
RWStructuredBuffer<uint> g_OutTriangleIndices : register(u1);

//HI-Z buffer

cbuffer cbPerFrame : register(b0){
	float4x4 g_ViewProj;
	float3 g_CamPos;
	float padding;
	float3 g_LightDir;
	float padding2;
};

cbuffer constants : register(b1){
	uint g_DrawArgIndex;
	uint g_BatchIndexOffset;
};

groupshared uint g_WorkGroupCount;

[numthreads(256,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex, uint3 disbatchThreadID : SV_DispatchThreadID ) {
	uint laneId = LaneId();

	if(groupIndex == 0){
		g_WorkGroupCount = 0;
	}

GroupMemoryBarrierWithGroupSync();
	uint index = disbatchThreadID.x;
	DrawCallArgs draw = g_DrawArgsBuffer[g_DrawArgIndex];

	//unpack triangle
	float4x4 wvp = mul(g_ViewProj, g_InputBuffer[draw.DrawIndex].World);

	uint indices[] = {	g_TriangleIndices[draw.IndexOffset + (index * 3)],
						g_TriangleIndices[draw.IndexOffset + (index * 3) + 1],
						g_TriangleIndices[draw.IndexOffset + (index * 3) + 2]};

	float4 v1 = mul( wvp, float4(g_VertexPositions[indices[0]], 1));
	float4 v2 = mul( wvp, float4(g_VertexPositions[indices[1]], 1));
	float4 v3 = mul( wvp, float4(g_VertexPositions[indices[2]], 1));

	//insert 2DH HERE

	//perspective divide
	v1.xyz /= v1.w;
	v2.xyz /= v2.w;
	v3.xyz /= v3.w;

	const Predicate laneActive = true;

	BitMask ballot = WaveBallot(laneActive);

	uint outCount = BitCount(ballot);
	uint localSlot = MBCount(ballot);

	uint waveSlot = 0;
	if(laneId == 0){
		InterlockedAdd(g_WorkGroupCount, outCount, waveSlot);
	}
	
	waveSlot = ReadFirstLaneUInt(waveSlot);

	if(laneActive){
		//write out triangle
		g_OutTriangleIndices[g_BatchIndexOffset + (localSlot + waveSlot) * 3] = indices[0];
		g_OutTriangleIndices[g_BatchIndexOffset + (localSlot + waveSlot) * 3 + 1] = indices[1];
		g_OutTriangleIndices[g_BatchIndexOffset + (localSlot + waveSlot) * 3 + 2] = indices[2];
	}
GroupMemoryBarrierWithGroupSync();

	if(groupIndex == 0){
		g_OutDrawArgs[g_DrawArgIndex] = draw;
		//g_OutDrawArgs[g_DrawArgIndex].IndexCount = g_WorkGroupCount;
		g_OutDrawArgs[g_DrawArgIndex].IndexOffset = g_BatchIndexOffset;
		g_OutDrawArgs[g_DrawArgIndex].Padding = outCount;
	}
}