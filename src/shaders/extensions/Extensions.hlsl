//map intrinsics to generic functions
#ifdef AMD_USE_SHADER_INTRINSICS

#include "extensions/ags_shader_intrinsics_dx12.hlsl"

typedef bool Predicate;
typedef uint2 BitMask;

#define ReadFirstLaneFloat(x) AmdExtD3DShaderIntrinsics_ReadfirstlaneF(x)
#define ReadFirstLaneUInt(x) AmdExtD3DShaderIntrinsics_ReadfirstlaneU(x)
#define ReadLaneFloat(x,y) AmdExtD3DShaderIntrinsics_ReadlaneF(x, y)
#define ReadLaneUInt(x,y) AmdExtD3DShaderIntrinsics_ReadlaneU(x, y)

#define WAVE_SIZE 64
//wrappers
float WaveMax(float val){
	return AmdExtD3DShaderIntrinsics_Max3F(val, 0, 0);
}

uint WaveMax(uint val){
	return AmdExtD3DShaderIntrinsics_Max3U(val, 0, 0);
}

float WaveMax(float3 val){
	return AmdExtD3DShaderIntrinsics_Max3F(val.x, val.y, val.z);
}

uint WaveMax(uint3 val){
	return AmdExtD3DShaderIntrinsics_Max3U(val.x, val.y, val.z);
}

uint LaneId(){
	return AmdExtD3DShaderIntrinsics_LaneId();
}

#define WaveBallot(x) AmdExtD3DShaderIntrinsics_Ballot(x)
#define WaveAny(x) AmdExtD3DShaderIntrinsics_BallotAny(x)
#define WaveAll(x) AmdExtD3DShaderIntrinsics_BallotAll(x)
#define MBCount(x) AmdExtD3DShaderIntrinsics_MBCnt(x)
//Naive BitCount think of other method later
uint BitCount32(uint bits) {
	uint count = 0;
	for (int i = 1; i < 32 + 1; i++)
		count += ((bits << (32 - i)) >> (32 - 1));
	return count;
}

uint BitCount(BitMask bm) {
	return BitCount32(bm.x) + BitCount32(bm.y);
}

#endif

#ifdef NVIDIA_USE_SHADER_INTRINSICS

#define NV_SHADER_EXTN_SLOT u1
#define NV_SHADER_EXTN_REGISTER_SPACE space10
#define WAVE_SIZE NV_WARP_SIZE
#include "extensions/nvHLSLExtns.h"

typedef int Predicate;
typedef uint BitMask;

#define WaveAll(x) NvAll(x)
#define WaveAny(x) NvAny(x)
#define WaveBallot(x) NvBallot(x)

uint LaneId(){
	return NvGetLaneId();
}

uint ReadFirstLaneUInt(uint data){
	return NvShfl(data, 0);
}

float ReadFirstLaneFloat(float data){
	return NvShfl(data, 0);
}

float WaveMax(float val){
	float waveMax = val;
	waveMax = max(waveMax, NvShflXor(waveMax, 16));
	waveMax = max(waveMax, NvShflXor(waveMax, 8));
	waveMax = max(waveMax, NvShflXor(waveMax, 4));
	waveMax = max(waveMax, NvShflXor(waveMax, 2));
	waveMax = max(waveMax, NvShflXor(waveMax, 1));
	return waveMax;
}

uint WaveMax(uint val){
	uint waveMax = val;
	waveMax = max(waveMax, NvShflXor(waveMax, 16));
	waveMax = max(waveMax, NvShflXor(waveMax, 8));
	waveMax = max(waveMax, NvShflXor(waveMax, 4));
	waveMax = max(waveMax, NvShflXor(waveMax, 2));
	waveMax = max(waveMax, NvShflXor(waveMax, 1));
	return waveMax;
}

float WaveMin(float val){
	float waveMin = val;
	waveMin = min(waveMin, NvShflXor(waveMin, 16));
	waveMin = min(waveMin, NvShflXor(waveMin, 8));
	waveMin = min(waveMin, NvShflXor(waveMin, 4));
	waveMin = min(waveMin, NvShflXor(waveMin, 2));
	waveMin = min(waveMin, NvShflXor(waveMin, 1));
	return waveMin;
}

uint WaveMin(uint val){
	uint waveMin = val;
	waveMin = min(waveMin, NvShflXor(waveMin, 16));
	waveMin = min(waveMin, NvShflXor(waveMin, 8));
	waveMin = min(waveMin, NvShflXor(waveMin, 4));
	waveMin = min(waveMin, NvShflXor(waveMin, 2));
	waveMin = min(waveMin, NvShflXor(waveMin, 1));
	return waveMin;
}
//Naive BitCount think of other method later
uint BitCount(BitMask bm){
	//might need to unroll
	uint count = 0;
	for(int i = 1; i < NV_WARP_SIZE + 1; i++){
		//8-bit example
		// 00110010 << 6
		// 10000000 >> 7
		// cnt += 00000001
		count += ((bm << (NV_WARP_SIZE - i)) >> (NV_WARP_SIZE - 1));
	}
	return count;
}
//Naive MBCount think of other method later
uint MBCount(uint laneId, BitMask bm){
	BitMask laneMask = (1 << laneId) - 1;
	laneMask &= bm;
	return BitCount(laneMask);
}

#endif