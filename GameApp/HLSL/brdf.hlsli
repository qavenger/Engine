#ifndef _BRDF_H
#define _BRDF_H
#include "ShaderCommon.hlsli"

float3 Diffuse_Lambert(float3 diffuseColor)
{
	return diffuseColor * kDIVPI;
}

float3 Diffuse_Burley(float3 diffuseColor, float roughness, float NoV, float NoL, float VoH)
{
	float FD90 = 0.5 + 2 * VoH * VoH * roughness;
	float FdV = 1 + (FD90 - 1) * pow5(1 - NoV);
	float FdL = 1 + (FD90 - 1) * pow5(1 - NoL);
	return diffuseColor	* kDIVPI * FdV * FdL;
}

float3 Diffuse_OrenNayar(float3 diffuseColor, float roughness, float aa, float nDotV, float nDotL, float vDotH)
{
	float vDotL = 2 * vDotH * vDotH - 1;
	float cosri = vDotL - nDotV * nDotL;
	float c1 = 1 - 0.5 * aa / (aa + 0.33);
	float c2 = 0.45 * aa / (aa + 0.09) * cosri * ( cosri >= 0 ? rcp(max ( nDotL, nDotV )) : 1 );
	return diffuseColor * kDIVPI * (c1 + c2)*(1 + roughness * 0.5);
}


float D_Beckmann(float aa, float NoH)
{
	float NoH2 = NoH*NoH;
	return exp( (NoH - 1) / (aa * NoH2 ) / (kPI * aa * NoH2 * NoH2));
}

// Normalized Distribution Function [GGX]
float D_GGX(float aa, float NoH)
{
	float d = (NoH * aa - NoH) * NoH + 1;
	return aa / (kPI*d*d);
}

float D_GGXAnsiotropic(float rX, float rY, float NoH, float3 H, float3 X, float3 Y)
{
	float ax = rX * rX;
	float ay = rY * rY;
	float XoH = dot(X, H);
	float YoH = dot(Y, H);
	float d = XoH*XoH / (ax*ax) + YoH*YoH / (ay*ay) + NoH*NoH;
	return 1 / (kPI * ax*ay * d*d);
}

float G_INV_Const()
{
	return 0.25;
}

float G_INV_Kelemen(float VoH)
{
	return rcp(4 * VoH * VoH + 1e-5);
}

float G_INV_Schlick(float a, float NoV, float NoL)
{
	float k = a * 0.5;
	float ik = 1 - k;
	float v = NoV * ik + k;
	float l = NoL * ik + k;
	return 0.25 / (v*l);
}

float G_INV_Smith(float aa, float NoV, float NoL)
{
	float v = NoV + sqrt(NoV * (NoV - NoV * aa) + aa);
	float l = NoL + sqrt(NoL * (NoL - NoL * aa) + aa);
	return rcp(v*l);
}

float G_INV_SmithJointApprox(float a, float NoV, float NoL)
{
	float ia = 1 - a;
	float v = NoL * (NoV * ia + a);
	float l = NoV * (NoL * ia + a);
	return 0.5 * rcp(v + l);
}

float3 F_Const(float3 specularColor)
{
	return specularColor;
}

float3 F_Schlick(float3 specularColor, float VoH)
{
	float term0 = pow5(1 - VoH);
	//return specularColor + (1 - specularColor) * term0;
	//return term0 + (1 - term0) * specularColor;
	return saturate(50 * specularColor.g) * term0 + (1 - term0) * specularColor;
}

float3 F_Fresnel(float3 specularColor, float VoH)
{
	float3 specSqrt = sqrt(clamp( float3(0,0,0), float3(0.99,0.99,0.99), specularColor ));
	float3 n = (1 + specSqrt) / (1 - specSqrt);
	float3 g = sqrt(n*n + VoH*VoH - 1);
	return 0.5 * square((g - VoH) / (g + VoH)) * 
		(1 + 
			square( 
			((g+VoH)*VoH - 1 ) / 
				((g-VoH)*VoH + 1) 
			)
		);
}



float3 BRDF_PI(float3 specularColor, float a, float3 L, float3 V, float3 N, float NoL, float NoV)
{
	float3 H = normalize(V + L);
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(V, H));
	float D = D_GGX(a*a, NoH);

	float3 F = F_Schlick(specularColor, VoH);

	float G_INV = G_INV_SmithJointApprox(a, NoV, NoL);

	return  D*G_INV*F;
}

#endif