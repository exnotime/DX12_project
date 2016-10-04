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

StructuredBuffer<DrawCallArgs> g_DrawArgsBuffer : register(t0);

RWStructuredBuffer<DrawCallArgs> g_OutDrawArgs : register(u0);

globallycoherent RWByteAddressBuffer counterBuffer : register(u1);

cbuffer constants : register(b0){
	uint g_DrawCallCount;
};

groupshared uint g_WorkGroupCount;
groupshared uint g_DisbatchSlot;

[numthreads(64,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex, uint3 disbatchThreadID : SV_DispatchThreadID ) {
	uint laneId = LaneId();

	if(groupIndex == 0){
		g_WorkGroupCount = 0;
		g_DisbatchSlot = 0;
	}
	uint index = disbatchThreadID.x;
GroupMemoryBarrierWithGroupSync();

	const Predicate laneActive = g_DrawArgsBuffer[index].IndexCount <= 300;

	BitMask ballot = WaveBallot(laneActive);

	uint outCount = BitCount(ballot);
	uint localSlot = MBCount(ballot);

	uint groupSlot = 0;
	if(laneId == 0){
		InterlockedAdd(g_WorkGroupCount, outCount, groupSlot);
	}
	
	groupSlot = ReadFirstLaneUInt(groupSlot);

	if(groupIndex == 0){
		counterBuffer.InterlockedAdd(0, g_WorkGroupCount, g_DisbatchSlot);
	}
GroupMemoryBarrierWithGroupSync();

	if(laneActive)
		g_OutDrawArgs[localSlot + groupSlot + g_DisbatchSlot] = g_DrawArgsBuffer[index];
}