
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

float4 PSMain(VSOut input) : SV_TARGET {
	return g_Tex.Sample(g_Sampler, input.texcoord);
}

