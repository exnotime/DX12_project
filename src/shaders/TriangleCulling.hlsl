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

Texture2D g_HIZBuffer : register(t4);
SamplerState g_Sampler : register(s0);

RWStructuredBuffer<DrawCallArgs> g_OutDrawArgs : register(u0);
RWStructuredBuffer<uint> g_OutTriangleIndices : register(u1);
//instrumentation
globallycoherent RWByteAddressBuffer g_CounterBuffer : register(u2);

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
#define INSTRUMENT 1

groupshared uint g_WorkGroupCount;

[numthreads(BATCH_SIZE, 1, 1)]
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
		float4x4 w = g_InputBuffer[draw.DrawIndex].World;

		uint indices[] = {	g_TriangleIndices[draw.IndexOffset + index],
							g_TriangleIndices[draw.IndexOffset + index + 1],
							g_TriangleIndices[draw.IndexOffset + index + 2]};

		float4 v1 = mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[0]], 1)));
		float4 v2 = mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[1]], 1)));
		float4 v3 = mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[2]], 1)));

		//backface
		float det = determinant(float3x3(v1.xyw,v2.xyw,v3.xyw));
		bool culled = det >= 0.0;
#ifdef INSTRUMENT
		if(culled){
			g_CounterBuffer.InterlockedAdd(8, 1);
		}
#endif
		//perspective divide
		v1.xyz /= v1.w;
		v2.xyz /= v2.w;
		v3.xyz /= v3.w;

		float3 vertexMax = max(v1.xyz, max(v2.xyz,v3.xyz));
		float3 vertexMin = min(v1.xyz, min(v2.xyz,v3.xyz));

		vertexMax.xy = vertexMax.xy * 0.5 + 0.5;
		vertexMin.xy = vertexMin.xy * 0.5 + 0.5;
		//occlusion
		if(!culled){
			float mipcount;
			float2 texDim;
			g_HIZBuffer.GetDimensions(0, texDim.x, texDim.y, mipcount);

			float2 edge1 = (v1.xy - v2.xy) * texDim;
			float2 edge2 = (v2.xy - v3.xy) * texDim;
			float2 edge3 = (v1.xy - v3.xy) * texDim;
			float longestEdge = max(length(edge1), max(length(edge2), length(edge3)));
			int mip = min(floor(log2(max(longestEdge, 1))), mipcount - 1);

			float depth1 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMin.x, 1.0 - vertexMin.y), mip).r;
			float depth2 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMax.x, 1.0 - vertexMin.y), mip).r;
			float depth3 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMin.x, 1.0 - vertexMax.y), mip).r;
			float depth4 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMax.x, 1.0 - vertexMax.y), mip).r;
			float maxDepth = max(max(depth1, depth2), max(depth3, depth4));

			culled = (vertexMin.z > maxDepth);
		}

		//small triangle
#ifdef INSTRUMENT
		if(!culled && any(round(vertexMin.xy * g_ScreenSize) == round(vertexMax.xy * g_ScreenSize))){
			g_CounterBuffer.InterlockedAdd(12, 1);
			culled = true;
		}
#else
		culled = culled || any(round(vertexMin.xy * g_ScreenSize) == round(vertexMax.xy * g_ScreenSize));
#endif

		//frustum
#ifdef INSTRUMENT
		if(!culled && (any(vertexMin.xy > 1) || any(vertexMax.xy < 0))){
			g_CounterBuffer.InterlockedAdd(16, 1);
			//culled = true;
		}
#else
		culled = culled || (any(vertexMin.xy > 1) || any(vertexMax.xy < 0));
#endif

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
		g_OutDrawArgs[g_BatchIndex + groupID.x].DrawIndex = draw.DrawIndex;
		g_OutDrawArgs[g_BatchIndex + groupID.x].MaterialIndex = draw.MaterialIndex;
		g_OutDrawArgs[g_BatchIndex + groupID.x].IndexCount = g_WorkGroupCount * 3;
		g_OutDrawArgs[g_BatchIndex + groupID.x].InstanceCount = 1;
		g_OutDrawArgs[g_BatchIndex + groupID.x].IndexOffset = indexOffset;

#ifdef INSTRUMENT
		//instrumentation
		g_CounterBuffer.InterlockedAdd(4, g_WorkGroupCount);
#endif

	}
}