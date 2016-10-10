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

globallycoherent RWByteAddressBuffer g_CounterBuffer : register(u1);

cbuffer constants : register(b0){
	uint g_DrawCallCount;
};

groupshared uint g_WorkGroupCount;
groupshared uint g_DisbatchSlot;

[numthreads(WAVE_SIZE,1,1)]
void CSMain(uint groupIndex : SV_GroupIndex, uint3 disbatchThreadID : SV_DispatchThreadID ) {
	uint laneId = LaneId();

	if(groupIndex == 0){
		g_WorkGroupCount = 0;
		g_DisbatchSlot = 0;
	}
	uint index = disbatchThreadID.x;
GroupMemoryBarrierWithGroupSync();

	Predicate laneActive = false;
	uint localSlot = 0;
	uint waveSlot = 0;

	if(index < g_DrawCallCount){
		laneActive = g_DrawArgsBuffer[index].DrawIndex & 1;
		BitMask ballot = WaveBallot(laneActive);

		uint outCount = BitCount(ballot);
		localSlot = MBCount(ballot);

		if(laneId == 0){
			InterlockedAdd(g_WorkGroupCount, outCount, waveSlot);
		}
		waveSlot = ReadFirstLaneUInt(waveSlot);
	}

	if(groupIndex == 0){
		g_CounterBuffer.InterlockedAdd(0, g_WorkGroupCount, g_DisbatchSlot);
	}
	if(laneActive){
		g_OutDrawArgs[localSlot + waveSlot + g_DisbatchSlot] = g_DrawArgsBuffer[index];
	}
}