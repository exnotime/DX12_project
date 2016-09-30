#include "extensions/Extensions.hlsl"

RWTexture2D<uint> outTex : register(u0);

[numthreads(8,8,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
	outTex[dispatchThreadID.xy] = WaveMax(LaneId());
}