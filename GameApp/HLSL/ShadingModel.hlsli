#ifndef _SHADINGMODEL_H
#define _SHADINGMODEL_H
#include "../../D3DGraphics/GraphicsDefine.h"
#include "ShaderCommon.hlsli"
#include "brdf.hlsli"

float3 StandardForward(float3 worldPos, float3 diffuseColor, float3 specularColor, float roughness, float3 V, float3 N, float shadowFactor)
{
	float NoL = 0;
	float NoV = saturate(abs(dot(N, V)) + 1e-5);
	float a = roughness * roughness;
	float attenuation = 1;
	float3 lightContribution = (float3)0;
	float3 color = (float3)0;
	float3 diffuse = Diffuse_Lambert(diffuseColor);
	LightInfo light = (LightInfo)0;
	for (uint i = 0; i < gNumLights; ++i)
	{
		light = lights[i];
		float3 lightColor = light.ColorIntensity.xyz * light.ColorIntensity.w;
		uint dirLightModifier = light.type == LIGHT_DIRECTIONAL;
		uint spotLightModifier = light.type == LIGHT_SPOT;
		float3 L = (light.PosRadius.xyz - worldPos) * (1 - dirLightModifier) -light.Direction * dirLightModifier;

		float lightDistSqr = dot(L, L);
		float lightDist = sqrt(lightDistSqr);
		L /= lightDist;

		NoL = saturate(dot(N, L));
		
		float distantRatio = saturate((lightDist - light.Falloff.z) / (light.PosRadius.w - light.Falloff.z));
		//float distantRatio = saturate(lightDistSqr / (light.PosRadius.w*light.PosRadius.w));
		float lightRadiusFilter = (1 - distantRatio * distantRatio);

		attenuation = (lightRadiusFilter / max(lightDistSqr, 1)) * (1 - spotLightModifier) + spotLightModifier * (lightRadiusFilter * lightRadiusFilter);

		float surfaceRatio = saturate(dot(-L, light.Direction));
		float coneAtt = (1 - saturate((light.Falloff.x - surfaceRatio) / (light.Falloff.x - light.Falloff.y))) * spotLightModifier + (1 - spotLightModifier);

		attenuation *= coneAtt;
		
		lightContribution = lightColor * NoL * (attenuation * (1 - dirLightModifier) + dirLightModifier) * (shadowFactor * (dirLightModifier) + (1 - dirLightModifier)) ;
		
	/*	float3 H = normalize(V + L);
		float VoH = saturate(dot(V, H));
		float3 diffuse = Diffuse_Burley(diffuseColor, roughness, NoV, NoL, VoH);
		*/
		//color += BRDF_PI(specularColor, a, L, V, N, NoL, NoV) * NoL;
		color += (diffuse + BRDF_PI(specularColor, a, L, V, N, NoL, NoV)) * lightContribution;
	}
	return color;
}


void StandardForward(float3 worldPos, float3 diffuseColor, float3 specularColor, float roughness, float3 V, float3 N, float shadowFactor, inout PS_OUTPUT output)
{
	float NoL = 0;
	float NoV = saturate(abs(dot(N, V)) + 1e-5);
	float a = roughness * roughness;
	float attenuation = 1;
	float3 lightContribution = (float3)0;
	float3 diffuse = Diffuse_Lambert(diffuseColor);
	LightInfo light = (LightInfo)0;
	float3 color = (float3)0;
	for (uint i = 0; i < gNumLights; ++i)
	{
		light = lights[i];
		float3 lightColor = light.ColorIntensity.xyz * light.ColorIntensity.w;
		uint dirLightModifier = light.type == LIGHT_DIRECTIONAL;
		uint spotLightModifier = light.type == LIGHT_SPOT;
		float3 L = (light.PosRadius.xyz - worldPos) * (1 - dirLightModifier) - light.Direction * dirLightModifier;

		float lightDistSqr = dot(L, L);
		float lightDist = sqrt(lightDistSqr);
		L /= lightDist;

		NoL = saturate(dot(N, L));

		float distantRatio = saturate((lightDist - light.Falloff.z) / (light.PosRadius.w - light.Falloff.z));
		//float distantRatio = saturate(lightDistSqr / (light.PosRadius.w*light.PosRadius.w));
		float lightRadiusFilter = (1 - distantRatio * distantRatio);

		attenuation = (lightRadiusFilter / max(lightDistSqr, 1)) * (1 - spotLightModifier) + spotLightModifier * (lightRadiusFilter * lightRadiusFilter);

		float surfaceRatio = saturate(dot(-L, light.Direction));
		float coneAtt = (1 - saturate((light.Falloff.x - surfaceRatio) / (light.Falloff.x - light.Falloff.y))) * spotLightModifier + (1 - spotLightModifier);

		attenuation *= coneAtt;

		lightContribution = lightColor * NoL * (attenuation * (1 - dirLightModifier) + dirLightModifier) * (shadowFactor * (dirLightModifier)+(1 - dirLightModifier));

		/*	float3 H = normalize(V + L);
		float VoH = saturate(dot(V, H));
		float3 diffuse = Diffuse_Burley(diffuseColor, roughness, NoV, NoL, VoH);
		*/
		//color += BRDF_PI(specularColor, a, L, V, N, NoL, NoV) * NoL;
		color += (diffuse + BRDF_PI(specularColor, a, L, V, N, NoL, NoV)) * lightContribution;
	}
	output.color.xyz = saturate(color);
	output.emissive.xyz = saturate(color - 1);
}


#endif