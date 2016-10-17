struct VSIn{
	float3 Position : POSITION;
};

cbuffer cbPerFrame : register(b0){
	float4x4 g_ViewProj;
	float3 g_CamPos;
	float padding;
	float3 g_LightDir;
	float padding2;
};

struct ShaderInput {
	float4x4 World;
	float4 Color;
};

StructuredBuffer<ShaderInput> g_InputBuffer : register(t0);

cbuffer cbShaderIndex : register(b2){
	uint g_ShaderIndex;
	uint g_MaterialIndex;
}

float4 VSMain(VSIn input) : SV_POSITION {
	float4x4 world = g_InputBuffer[g_ShaderIndex].World;
	float4x4 wvp = mul(g_ViewProj, world);
	return mul(wvp,float4(input.Position, 1));
}