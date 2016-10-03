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

globallycoherent RWBuffer<uint> g_Counter : register(u1);

cbuffer constants : register(b0){
	uint g_DrawCallCount;
};

groupshared uint g_WorkGroupSlot;

[numthreads(WAVE_SIZE,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex ) {
	uint laneId = LaneId();

	if(groupIndex == 0)
		g_WorkGroupSlot = 0;

GroupMemoryBarrierWithGroupSync();

	const Predicate laneActive = g_DrawArgsBuffer[groupIndex].IndexCount <= 36;

	BitMask ballot = WaveBallot(laneActive);

	uint outCount = BitCount(ballot);
	uint localSlot = MBCount(ballot);

	uint groupSlot = 0;
	if(laneId == 0){
		InterlockedAdd(g_WorkGroupSlot, outCount, groupSlot);
	}
	
	groupSlot = ReadFirstLaneUInt(groupSlot);

GroupMemoryBarrierWithGroupSync();

	if(groupIndex == 0){
		InterlockedAdd(g_Counter[0], groupSlot, g_WorkGroupSlot);
		g_Counter[0] += groupSlot;
	}

	if(laneActive)
		g_OutDrawArgs[localSlot + groupSlot + g_WorkGroupSlot] = g_DrawArgsBuffer[groupIndex];
}