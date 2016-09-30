//map intrinsics to generic functions
#ifdef AMD_USE_SHADER_INTRINSICS

#include "extensions/ags_shader_intrinsics_dx12.hlsl"

typedef bool Predicate;
typedef uint2 BitMask;

#define ReadFirstLineFloat(x) AmdExtD3DShaderIntrinsics_ReadfirstlaneF(x)
#define ReadFirstLineUInt(x) AmdExtD3DShaderIntrinsics_ReadfirstlaneU(x)
#define ReadLaneFloat(x,y) AmdExtD3DShaderIntrinsics_ReadlaneF(x, y)
#define ReadLaneUInt(x,y) AmdExtD3DShaderIntrinsics_ReadlaneU(x, y)

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

#endif

#ifdef NVIDIA_USE_SHADER_INTRINSICS

#define NV_SHADER_EXTN_SLOT u1
#define NV_SHADER_EXTN_REGISTER_SPACE space10

#include "nvHLSLExtns.hlsl"

typedef Predicate int;
typedef BitMask uint;

#define WaveAll(x) NvAll(x)
#define WaveAny(x) NvAny(x)
#define WaveBallot(x) NvBallot(x)
#define LaneId() NvGetLaneId()

Scalar WaveMax(Scalar val){
	Scalar waveMax = val;
	waveMax = max(waveMax, NvShflXor(waveMax, 16));
	waveMax = max(waveMax, NvShflXor(waveMax, 8));
	waveMax = max(waveMax, NvShflXor(waveMax, 4));
	waveMax = max(waveMax, NvShflXor(waveMax, 2));
	waveMax = max(waveMax, NvShflXor(waveMax, 1));
	return waveMax;
}

Vector WaveMax(Vector val){
	Vector waveMax = val;
	waveMax = max(waveMax, NvShflXor(waveMax, 16));
	waveMax = max(waveMax, NvShflXor(waveMax, 8));
	waveMax = max(waveMax, NvShflXor(waveMax, 4));
	waveMax = max(waveMax, NvShflXor(waveMax, 2));
	waveMax = max(waveMax, NvShflXor(waveMax, 1));
	return waveMax;
}

Scalar WaveMin(Scalar val){
	Scalar waveMin = val;
	waveMin = min(waveMin, NvShflXor(waveMin, 16));
	waveMin = min(waveMin, NvShflXor(waveMin, 8));
	waveMin = min(waveMin, NvShflXor(waveMin, 4));
	waveMin = min(waveMin, NvShflXor(waveMin, 2));
	waveMin = min(waveMin, NvShflXor(waveMin, 1));
	return waveMin;
}

Vector WaveMin(Vector val){
	Vector waveMin = val;
	waveMin = min(waveMin, NvShflXor(waveMin, 16));
	waveMin = min(waveMin, NvShflXor(waveMin, 8));
	waveMin = min(waveMin, NvShflXor(waveMin, 4));
	waveMin = min(waveMin, NvShflXor(waveMin, 2));
	waveMin = min(waveMin, NvShflXor(waveMin, 1));
	return waveMin;
}

//Naive MBCount think of other method later
uint MBCount(BitMask bm){
	//might need to unroll
	uint count = 0;
	for(int i = 1; i < NV_WARP_SIZE + 1; i++){
		//8-bit example
		// 00110010 << 6
		// 10000000 >> 7
		// cnt += 00000001
		count += ((NV_WARP_SIZE - 1) >> ((NV_WARP_SIZE - i) << bm)) ;
	}
	return count;
}

#endif