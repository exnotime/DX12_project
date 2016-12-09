struct VSIn{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
	uint InstanceID : SV_InstanceID;
};
struct PSIn{
	float4 position : SV_POSITION;
	float3 posW : POSW;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
	float3 color : COLOR;
};

Texture2D g_Materials[4000] : register(t0,space1);

TextureCube g_DiffEnvMap : register(t1);
TextureCube g_SpecEnvMap : register(t2);
Texture2D g_IBLTex : register(t3);

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

#define sqr(x) x * x
#define PI 3.141592653589793
#define M_INV_PI 0.31830988618379067153776752674503
#define MOD3 float3(443.8975,397.2973, 491.1871)
#define EPS 0.0001
#define GAMMA 1.0

float Visibillity(float roughness, float ndotv, float ndotl){
	float m2 = sqr(roughness);
	float visV = ndotl * sqrt(ndotv * (ndotv - ndotv * m2) + m2);
	float visL = ndotv * sqrt(ndotl * (ndotl - ndotl * m2) + m2);
	return 0.5 / max(visV + visL, 0.00001);
}

float GeometricAtt(float hdotn, float vdotn, float vdoth, float ndotl)
{
	float a = (2 * hdotn * vdotn) / vdoth;
	float b = (2 * hdotn * ndotl) / vdoth;
	float c = min(a,b);
	return min(1,c);
}

float Distrobution(float roughness, float ndoth){
	float m2 = sqr(roughness);
	float d = (ndoth * m2 - ndoth) * ndoth + 1.0;
	return m2 / (d * d* PI);
}

float GGX_D(float roughness, float ndoth){
	float rSqr = sqr(roughness);
	float rSqr2 = sqr(rSqr);
	float t = sqr(ndoth) * (rSqr2 - 1) + 1;
	return rSqr2 / (PI * t * t);
}

float3 Fresnel(float3 specular, float vdoth){
	return saturate(50 * specular) * specular + (1.0 - specular) * pow((1.0 - vdoth),5.0);
}

float3 F_Schlick(float3 f0, float vdoth){
	return f0 + (1.0 - f0) * pow(1.0f - vdoth, 5.0f);
}

float IORToF0(float n){
	return pow(n - 1, 2) / pow(n + 1, 2);
}

float G1V ( float dotNV, float k ) {
	return 1.0 / (dotNV*(1.0 - k) + k);
}

float3 CookTorranceSpecular(float3 normal, float3 lightDir, float3 toEye, float roughness, float metal, float3 baseColor) {
	float3 halfWayVector = normalize(toEye + lightDir);
	float hdotl = saturate(dot(halfWayVector, lightDir));
	float ndotl = saturate(dot(normal, lightDir));
	float ndoth = saturate(dot(halfWayVector, normal));
	float ndotv = saturate(dot(toEye, normal));
	float vdoth = saturate(dot(halfWayVector, toEye));

	float ior = lerp(1.5, 3.0, metal);
	float f0 = IORToF0(ior);
	float3 F0 = float3(f0,f0,f0);
	F0 = lerp(F0, F0 * baseColor, metal);

	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;

	float denom = ndoth * ndoth * (alphaSqr - 1.0) + 1.0;
	float D = alphaSqr / (PI * denom * denom);

	float3 F = F_Schlick(F0, hdotl);

	float k = alpha / 2.0;
	float vis = G1V(ndotl, k) * G1V(ndotv, k);

	return D * F * vis;
}

float3 CalcDirLight(float3 albedo, float3 normal, float3 toEye, float roughness, float metallic) {
	float3 lightDir = -normalize(g_LightDir);
	float3 spec = float3(0,0,0);
	float3 diff = float3(0,0,0);
	float ndotl = dot(normal, lightDir);
	diff = max(ndotl, 0) * albedo;
	if(ndotl > 0){
		spec = CookTorranceSpecular(normal, lightDir, toEye, roughness, metallic, albedo);
		spec = saturate(spec);
	}
	diff = lerp(diff, float3(0.0,0.0,0.0), metallic);
	spec = lerp(spec, spec * albedo, metallic);
	return (diff + spec) * 0.7;
}

float3 AproximateIBLSpecular(float3 F0 , float roughness, float3 normal, float3 toeye){
 	float NoV = saturate(dot(normal, toeye));
 	NoV = max(0.001, NoV);
 	float3 R = 2 * dot(normal, toeye) * normal - toeye;

 	uint2 texDim;
 	uint levels;
 	g_SpecEnvMap.GetDimensions(0, texDim.x, texDim.y,levels);
	float numMips = ceil(log2(float(max(texDim.x,texDim.y)))) - 1.0f;
	float mipLevel = numMips * roughness;

	float3 color = pow(g_SpecEnvMap.SampleLevel(g_sampler, R, mipLevel).rgb, GAMMA);
	float2 envBRDF = g_IBLTex.Sample(g_sampler, float2(roughness, NoV)).rg;

	return color * (envBRDF.x * F0 + envBRDF.y);
 }

float3 CalcIBLLight( float3 inNormal, float3 toeye, float3 baseColor, float roughness, float metal)
{
	float4 lightColor = float4(0,0,0,1);
	float ior = lerp(1.4, 3.0, metal);//ior for plastic and iron, metallic values should actually be fetched from a texture and be in rgb space but this will do for now
	float f0 = IORToF0(ior);
	float3 F0 = float3(f0,f0,f0);
	F0 = lerp(F0, F0 * baseColor, metal);

 	float3 irradiance = pow(g_DiffEnvMap.Sample(g_sampler, inNormal).rgb, GAMMA);
 	float3 diffuse = baseColor * irradiance;
 	float3 specular = AproximateIBLSpecular(F0, roughness, inNormal, toeye);
 	specular = saturate(specular);
	return float3(specular + diffuse * (float3(1,1,1) - F0) * (1 - metal));
}

float3 hash32(float2 p){
	float3 p3 = frac(float3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return frac(float3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float3 ditherRGB(float3 c, float2 seed){
	return c + hash32(seed) / 255.0;
}

float3 CalcBumpedNormal(float3 Bump, float3 Normal, float3 Tangent){
	float3 normal = normalize(Normal);
	float3 tangent = normalize(Tangent);
	//tangent = normalize(tangent - dot(tangent,normal) * normal);
	float3 bitangent = normalize(cross(tangent,normal));
	float3 bump = normalize((Bump * 2.0) - 1.0);
	float3x3 TBN = float3x3(tangent,bitangent,normal);
	float3 newNormal = mul(bump, TBN);
	return normalize(newNormal);
}

PSIn VSMain(VSIn input){
	PSIn result;
	float4x4 world = g_InputBuffer[g_ShaderIndex + input.InstanceID].World;
	float4x4 wvp = mul(g_ViewProj, world);
	result.posW = mul(world, float4(input.Position,1)).xyz;
	result.position = mul(wvp,float4(input.Position,1));
	result.uv = input.TexCoord;
	result.normal = mul(world, float4(input.Normal,0)).xyz;
	result.tangent = mul(world, float4(input.Tangent,0)).xyz;
	result.color = g_InputBuffer[g_ShaderIndex + input.InstanceID].Color.rgb;
	return result;
}

float4 PSMain(PSIn input) : SV_TARGET {
	float4 color = float4(0,0,0,1);
	float4 albedo = float4(input.color,1) * pow(g_Materials[NonUniformResourceIndex(g_MaterialIndex)].Sample(g_sampler, input.uv), GAMMA);
	clip( albedo.a < 0.1f ? -1:1 );

	float r = 1.0 - pow(g_Materials[NonUniformResourceIndex(g_MaterialIndex+2)].Sample(g_sampler, input.uv).r, GAMMA);
	float m = pow(g_Materials[NonUniformResourceIndex(g_MaterialIndex+3)].Sample(g_sampler, input.uv).r, GAMMA);
	float3 normal = CalcBumpedNormal(g_Materials[NonUniformResourceIndex(g_MaterialIndex+1)].Sample(g_sampler, input.uv).xyz, input.normal, input.tangent);
	float3 toeye = normalize( g_CamPos - input.posW );

	color.rgb = CalcDirLight(albedo.rgb, normal, toeye, r, m);
	color.rgb += CalcIBLLight(normal, toeye, albedo.rgb, r, m);
	color.rgb = ditherRGB(color.rgb, input.position.xy);
	color.rgb = pow(color.rgb, 1.0 / GAMMA);
	return float4(color.rgb,1);
}
