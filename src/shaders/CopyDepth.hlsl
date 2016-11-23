Texture2D inTex : register(t0);
RWTexture2D<float> outTex : register(u0);

cbuffer constants : register(b0){
	float width;
	float height;
};

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID){
	if(dispatchThreadID.x >= uint(width) || dispatchThreadID.y >= uint(height))
		return;
	outTex[dispatchThreadID.xy] = inTex[dispatchThreadID.xy].r;
}