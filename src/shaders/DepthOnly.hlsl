struct VSIn{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
	uint InstanceID : SV_InstanceID;
};

struct PSIn{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D g_Materials[4000] : register(t0,space1);

SamplerState g_sampler : register(s0);

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

PSIn VSMain(VSIn input) {
	PSIn result;
	float4x4 world = g_InputBuffer[g_ShaderIndex + input.InstanceID].World;
	float4x4 wvp = mul(g_ViewProj, world);
	result.position = mul(wvp,float4(input.Position, 1));
	//result.position.z += 0.001;
	result.uv = input.TexCoord;
	return result;
}

float4 PSMain(PSIn input) : SV_TARGET {
	//float4 albedo = g_Materials[NonUniformResourceIndex(g_MaterialIndex)].Sample(g_sampler, input.uv);
	//clip( albedo.a <= 0.1 ? -1 : 1 );
	return float4(0,0,0,1);
}