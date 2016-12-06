
struct VSOut
{
	float4 position_cs : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VSOut VSMain(uint id: SV_VertexID)
{
	VSOut output;

	output.texcoord = float2((id << 1) & 2, id & 2);
	output.position_cs = float4(output.texcoord * float2(2, -2) + float2(-1, 1), 0, 1);

	return output;
}


Texture2D g_Tex : register(t0);
SamplerState g_Sampler : register(s0);

#define MOD3 float3(443.8975,397.2973, 491.1871)
float3 hash32(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return frac(float3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float3 ditherRGB(float3 c, float2 seed){
	return c + hash32(seed) / 255.0;
}

float4 PSMain(VSOut input) : SV_TARGET {
	float depth = g_Tex.Sample(g_Sampler, input.texcoord).r;
	float f = 150.0;
	float n = 0.5;
	float ez = (n * f) / (f - depth * (f - n));
	depth = (ez - n) / (f - n);
	//dither to get rid of banding
	return float4( ditherRGB(float3(depth, depth, depth), input.position_cs.xy), 1);
}

