
struct VSIn{
	float3 Position : POSITION;
};

cbuffer cbPerFrame : register(b0){
	float4x4 g_ViewProj;
	float4 g_Color[16];
};

cbuffer constans : register(b1){
	uint lineIndex;
};

float4 VSMain(VSIn input) : SV_POSITION {
	return mul(g_ViewProj, float4(input.Position,1.0));
}

float4 PSMain() : SV_TARGET {
	return g_Color[lineIndex];
}