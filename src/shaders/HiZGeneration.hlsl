RWTexture2D<float> inTex : register(u0);
RWTexture2D<float> outTex : register(u1);

float GetDepth(int2 pos){
	return inTex.Load(pos);
}

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID){
	float4 texValues;
	int2 inputPos = dispatchThreadID.xy * 2;

	texValues.x = GetDepth(inputPos);
	texValues.y = GetDepth(inputPos + int2(1, 0));
	texValues.z = GetDepth(inputPos + int2(1, 1));
	texValues.w = GetDepth(inputPos + int2(0, 1));

	float outVal = max(max(texValues.x, texValues.y), max(texValues.z,texValues.w));
	outTex[dispatchThreadID.xy] = outVal;
}