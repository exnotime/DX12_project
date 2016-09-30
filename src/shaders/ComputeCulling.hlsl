#include "extensions/Extensions.hlsl"

#struct DrawCallArgs{
	uint DrawIndex;
	uint MaterialIndex;

	uint IndexCount;
	uint InstanceCount;
	uint IndexOffset;
	uint BaseVertex;
	uint BaseInstance;
};

StructuredBuffer<DrawCallArgs> g_DrawArgsBuffer : register(t0);

RWStructuredBuffer<DrawCallArgs> g_OutDrawArgs : register(u0);

RWBuffer AtomicBuffer : register(u1){
	uint g_GlobalSlot;
};

cbuffer constants : register(b0){
	uint g_DrawCallCount;
};

groupshared uint g_WorkGroupSlot;

[numthreads(64,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex ) {
	uint laneId = LaneId();

	if(groupIndex == 0)
		g_WorkGroupSlot = 0;

	if(groupIndex < g_DrawCallCount) {
		
		const Predicate laneActive = g_DrawArgsBuffer[groupIndex].IndexCount < 30;

		BitMask ballot = WaveBallot(laneActive);

		uint outCount = BitCount(ballot);
		uint localSlot = MBCount(ballot);
		uint groupSlot;
		if(laneId == 0){
			InterlockedAdd(g_WorkGroupSlot, outCount, groupSlot);
		}
		
		ReadFirstLane(groupSlot);

		if(groupIndex == 0)
			InterlockedAdd(g_GlobalSlot, outCount, groupSlot);

		g_OutDrawArgs[localSlot + groupSlot + g_GlobalSlot] = g_DrawArgsBuffer[groupIndex];
	}


}