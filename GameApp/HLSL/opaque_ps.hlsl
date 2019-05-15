#include "../../D3DGraphics/GraphicsDefine.h"
#include "ShadingModel.hlsli"
#include "ShaderCommon.hlsli"

Texture2D gTextures[2]			: register(t1);

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	float3x3 TBN = (float3x3)0;
	TBN[0] = normalize(input.tangent);
	TBN[1] = normalize(input.bitangent);
	TBN[2] = normalize(input.normal);
	//float3 viewDir = normalize_fast(input.viewDir);
	float3 viewDir = normalize(input.viewDir);

	float4 albedo = gTextures[0].Sample(gAnisotropicWrap, input.uv);
	float4 surfaceInfo = gTextures[1].Sample(gAnisotropicWrap, input.uv);
	float3 base = pow(abs(albedo.xyz), 2.2);
	//float3 base = albedo.xyz;

	float metallic = surfaceInfo.w;
	float3 diffuse = base - base * metallic;
	float3 specular = (0.04 - 0.04 * metallic) + base * metallic;
	//float4 color = albedo;
	output.color = albedo;
	float3 normal = surfaceInfo.xyz;
	normal = remapSimpleFrom01(normal);
	normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
	normal = mul(normal, TBN);
	float shadowFactor = CalcShadowFactor(input.shadowPos);
	
	StandardForward(input.worldpos, diffuse, specular, surfaceInfo.z, viewDir, normal, shadowFactor, output);
	
	//output.color = saturate(color);
	/*float2 uv = remap(input.uv, 0, 1, -kPI, kPI);
	float gradient = atan2(uv.y, uv.x);
	gradient = remapSimpleTo01(gradient);*/
	
	return output;
}