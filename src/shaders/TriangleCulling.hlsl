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
	float2 g_ScreenSize;
};

cbuffer constants : register(b1){
	uint g_BatchIndex; //what batch this disbatch starts with
	uint g_BatchDrawId; //what draw id this disbatch is culling
};

#define BATCH_SIZE 256

groupshared uint g_WorkGroupCount;

[numthreads(BATCH_SIZE,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex, uint3 disbatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID ) {
	uint laneId = LaneId();

	if(groupIndex == 0){
		g_WorkGroupCount = 0;
	}

GroupMemoryBarrierWithGroupSync();

	uint index = (groupIndex + (groupID.x * BATCH_SIZE)) * 3;
	uint waveSlot = 0;
	uint localSlot = 0;
	uint outCount = 0;
	uint indexOffset = (groupID.x + g_BatchIndex) * BATCH_SIZE * 3;

	DrawCallArgs draw = g_DrawArgsBuffer[g_BatchDrawId];

	if(index < draw.IndexCount){
		//unpack triangle
		float4x4 wvp = mul(g_ViewProj, g_InputBuffer[draw.DrawIndex].World);

		uint indices[] = {	g_TriangleIndices[draw.IndexOffset + index],
							g_TriangleIndices[draw.IndexOffset + index + 1],
							g_TriangleIndices[draw.IndexOffset + index + 2]};

		float4 v1 = mul( wvp, float4(g_VertexPositions[indices[0]], 1));
		float4 v2 = mul( wvp, float4(g_VertexPositions[indices[1]], 1));
		float4 v3 = mul( wvp, float4(g_VertexPositions[indices[2]], 1));

		//backface
		float det = determinant(float3x3(v1.xyw,v2.xyw,v3.xyw));
		bool culled = det >= 0.0;

		//perspective divide
		v1.xyz /= v1.w;
		v2.xyz /= v2.w;
		v3.xyz /= v3.w;

		float3 vertexMax = max(v1.xyz, max(v2.xyz,v3.xyz));
		float3 vertexMin = min(v1.xyz, min(v2.xyz,v3.xyz));

		vertexMax.xy = vertexMax.xy * 0.5 + 0.5;
		vertexMin.xy = vertexMin.xy * 0.5 + 0.5;
		//small triangle
		culled = culled || any(round(vertexMin.xy * g_ScreenSize) == round(vertexMax.xy * g_ScreenSize));
		//frustum
		culled = culled || (any(vertexMin.xy > 1) || any(vertexMax.xy < 0) || vertexMin.z < 0);

		const Predicate laneActive = !culled;
		BitMask ballot = WaveBallot(laneActive);

		outCount = BitCount(ballot);
		localSlot = MBCount(ballot);

		if(laneId == 0){
			InterlockedAdd(g_WorkGroupCount, outCount, waveSlot);
		}
		
		waveSlot = ReadFirstLaneUInt(waveSlot);

		if(laneActive){
			//write out triangle
			g_OutTriangleIndices[indexOffset + (localSlot + waveSlot) * 3] = indices[0];
			g_OutTriangleIndices[indexOffset + (localSlot + waveSlot) * 3 + 1] = indices[1];
			g_OutTriangleIndices[indexOffset + (localSlot + waveSlot) * 3 + 2] = indices[2];
		}
	}
GroupMemoryBarrierWithGroupSync();

	if(groupIndex == 0){
		g_OutDrawArgs[g_BatchIndex + groupID.x] = draw;
		g_OutDrawArgs[g_BatchIndex + groupID.x].IndexCount = g_WorkGroupCount * 3;
		g_OutDrawArgs[g_BatchIndex + groupID.x].IndexOffset = indexOffset;
	}
}