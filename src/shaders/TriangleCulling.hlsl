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

globallycoherent RWStructuredBuffer<DrawCallArgs> g_OutDrawArgs : register(u0);
RWStructuredBuffer<uint> g_OutTriangleIndices : register(u1);
globallycoherent RWByteAddressBuffer g_CullingStats : register(u2);
globallycoherent RWByteAddressBuffer g_Counters : register(u3);

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
	uint g_BatchIndexOffset; //where in the index buffer should start writing
	uint g_BatchOutDrawId;
};

groupshared uint g_WorkGroupCount;
groupshared uint g_GroupSlot;

//#define TRIANGLE_COUNT 1
#define TRIANGLE_SIZE 3 * TRIANGLE_COUNT

bool CullTriangle(float4 vertices[3], uint3 indices){
	bool culled = false;

#ifdef INSTRUMENT
		g_CullingStats.InterlockedAdd(0, 1);
#endif

#ifdef FILTER_BACKFACE
	//backface
	float det = determinant(float3x3(	vertices[0].xyw,
										vertices[1].xyw,
										vertices[2].xyw));
	culled = det >= 0.0;
	#ifdef INSTRUMENT
	if(culled)
		g_CullingStats.InterlockedAdd(4, 1);
	#endif
#endif

	int verticesInFrontOfNearPlane = 0;
	for(int i = 0; i < 3; ++i){
		//perspective divide
		vertices[i].xyz /= vertices[i].w;
		vertices[i].xy /= 2;
		vertices[i].xy += float2(0.5, 0.5);
		if(vertices[i].w < 0)
			verticesInFrontOfNearPlane++;
	}

	float3 vertexMax = max(vertices[0].xyz, max(vertices[1].xyz,vertices[2].xyz));
	float3 vertexMin = min(vertices[0].xyz, min(vertices[1].xyz,vertices[2].xyz));

#ifdef FILTER_SMALL_TRIANGLE
	//small triangle
	bool cullSmallTriangle = any(round(vertexMin.xy * g_ScreenSize) == round(vertexMax.xy * g_ScreenSize));
	#ifdef INSTRUMENT
	if(!culled && cullSmallTriangle)
		g_CullingStats.InterlockedAdd(8, 1);
	#endif
	culled = culled || cullSmallTriangle;
#endif

#ifdef FILTER_FRUSTUM
	//frustum
	bool cullfrustum = false;
	if(verticesInFrontOfNearPlane == 3)
		cullfrustum = true;
	if(verticesInFrontOfNearPlane == 0){
		cullfrustum = (any(vertexMin.xy > 1) || any(vertexMax.xy < 0));
	}
	#ifdef INSTRUMENT
	if(!culled && cullfrustum)
		g_CullingStats.InterlockedAdd(12, 1);
	#endif
	culled = culled || cullfrustum;
#endif

#ifdef FILTER_OCCLUSION
	//occlusion
	if(!culled){
		float mipcount;
		float2 texDim;
		g_HIZBuffer.GetDimensions(0, texDim.x, texDim.y, mipcount);
		float2 edge1 = (vertices[0].xy - vertices[1].xy) * texDim;
		float2 edge2 = (vertices[1].xy - vertices[2].xy) * texDim;
		float2 edge3 = (vertices[0].xy - vertices[2].xy) * texDim;
		float longestEdge = max(length(edge1), max(length(edge2), length(edge3)));
		int mip = min(ceil(log2(max(longestEdge, 1))) - 1, mipcount - 1);
		float depth1 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMin.x, 1.0 - vertexMin.y), mip).r;
		float depth2 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMax.x, 1.0 - vertexMin.y), mip).r;
		float depth3 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMin.x, 1.0 - vertexMax.y), mip).r;
		float depth4 = g_HIZBuffer.SampleLevel(g_Sampler, float2(vertexMax.x, 1.0 - vertexMax.y), mip).r;
		float maxDepth = max(max(depth1, depth2), max(depth3, depth4));
		culled = (vertexMin.z > maxDepth);
		#ifdef INSTRUMENT
		if(culled)
			g_CullingStats.InterlockedAdd(16, 1);
		#endif
	}
#endif
	return culled;
}
[numthreads(BATCH_SIZE, 1, 1)]
void CSMain(uint groupIndex : SV_GroupIndex, uint3 disbatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID ) {
	if(groupIndex == 0){
		g_WorkGroupCount = 0;
		g_GroupSlot = 0;

		if(groupID.x == 0){
			g_OutDrawArgs[g_BatchOutDrawId].DrawIndex = g_DrawArgsBuffer[g_BatchDrawId].DrawIndex;
			g_OutDrawArgs[g_BatchOutDrawId].MaterialIndex = g_DrawArgsBuffer[g_BatchDrawId].MaterialIndex;
			g_OutDrawArgs[g_BatchOutDrawId].InstanceCount = 1;
			g_OutDrawArgs[g_BatchOutDrawId].BaseVertex = 0;
			g_OutDrawArgs[g_BatchOutDrawId].BaseInstance = 0;
			//g_OutDrawArgs[g_BatchOutDrawId].IndexCount = 0;
			g_OutDrawArgs[g_BatchOutDrawId].IndexOffset = g_BatchIndex * BATCH_SIZE * TRIANGLE_SIZE;
			g_Counters.Store(g_BatchIndex * 4, 0);
		}
	}
GroupMemoryBarrierWithGroupSync();
	const uint laneId = LaneId();
	const uint index = (groupIndex + (groupID.x * BATCH_SIZE) + g_BatchIndexOffset) * TRIANGLE_SIZE;
	const uint indexOffset = g_BatchIndex * BATCH_SIZE * TRIANGLE_SIZE;
	uint waveSlot = 0;
	uint localSlot = 0;
	
	uint3 indices[TRIANGLE_COUNT];
	//DrawCallArgs draw = g_DrawArgsBuffer[g_BatchDrawId];
	Predicate laneActive = false;

	if(index < g_DrawArgsBuffer[g_BatchDrawId].IndexCount){
		//unpack triangles
		float4x4 w = g_InputBuffer[g_DrawArgsBuffer[g_BatchDrawId].DrawIndex].World;
		bool4 culledTris = bool4(false, false, false, false);

		[unroll]
		for(int i = 0; i < TRIANGLE_COUNT; ++i){
			const uint k = g_DrawArgsBuffer[g_BatchDrawId].IndexOffset + index + i * 3;

			indices[i][0] = g_TriangleIndices[k];
			indices[i][1] = g_TriangleIndices[k + 1];
			indices[i][2] = g_TriangleIndices[k + 2];

			float4 vertices[] = {
			mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[i][0]], 1))),
			mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[i][1]], 1))),
			mul(g_ViewProj, mul( w, float4(g_VertexPositions[indices[i][2]], 1)))};

			culledTris[i] = !CullTriangle(vertices, indices[i]);
		}

		laneActive = any(culledTris);
		BitMask ballot = WaveBallot(laneActive);

		uint outCount = BitCount(ballot);
		localSlot = MBCount(ballot);

		if(laneId == 0){
			InterlockedAdd(g_WorkGroupCount, outCount, waveSlot);
		}
		
		waveSlot = ReadFirstLaneUInt(waveSlot);
	}

GroupMemoryBarrierWithGroupSync();
	if(groupIndex == 0){
		g_Counters.InterlockedAdd(g_BatchOutDrawId * 4, g_WorkGroupCount, g_GroupSlot);
		InterlockedAdd(g_OutDrawArgs[g_BatchOutDrawId].IndexCount, g_WorkGroupCount * TRIANGLE_SIZE);
	#ifdef INSTRUMENT
		g_CullingStats.InterlockedAdd(20, g_WorkGroupCount);
	#endif
	}

GroupMemoryBarrierWithGroupSync();
	if(laneActive){
		//write out triangles
		[unroll]
		for(int i = 0; i < TRIANGLE_COUNT; ++i){
			uint outputIndex = indexOffset + (localSlot + waveSlot + g_GroupSlot) * TRIANGLE_SIZE + i * 3;
			g_OutTriangleIndices[outputIndex] = indices[i][0];
			g_OutTriangleIndices[outputIndex + 1] = indices[i][1];
			g_OutTriangleIndices[outputIndex + 2] = indices[i][2];
		}
	}
}