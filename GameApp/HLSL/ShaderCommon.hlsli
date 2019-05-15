#ifndef _SHADERCOMMON_H
#define _SHADERCOMMON_H
#define kFLT_MAX		3.402823466e+38F
#define kDIVPI		0.318309886f
#define kPI			3.141592654f
#define kPIDIV2		1.570796327f
#define kPIDIV4		0.785398163f
#define kEpsilon    1e-5

Texture2D gCustomMap			: register(t0);

SamplerState gPointWrap			: register(s0);
SamplerState gPointClamp		: register(s1);
SamplerState gLinearWrap		: register(s2);
SamplerState gLinearClamp		: register(s3);
SamplerState gAnisotropicWrap	: register(s4);
SamplerState gAnisotropicClamp	: register(s5);
SamplerComparisonState gSamShadow : register(s6);

float CalcShadowFactor(float3 pos)
{
	uint w, h, numMips;
	gCustomMap.GetDimensions(0, w, h, numMips);
	float dx = 1.0 / w;

	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0, -dx), float2(dx, -dx),
		float2(-dx,   0), float2(0, 0  ), float2(dx,   0),
		float2(-dx,  dx), float2(0, dx ), float2(dx,  dx)
	};

	float sum = 0;
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		sum += gCustomMap.SampleCmpLevelZero(gSamShadow, pos.xy + offsets[i], pos.z).r;
	}
	return sum * 0.111111111;
}

float square(float x)
{
	return x * x;
}

float2 square(float2 x)
{
	return x * x;
}

float3 square(float3 x)
{
	return x * x;
}

float4 square(float4 x)
{
	return x * x;
}

float pow5(float x)
{
	float xx = x * x;
	return xx*xx*x;
}

float2 pow5(float2 x)
{
	float2 xx = x * x;
	return xx*xx*x;
}

float3 pow5(float3 x)
{
	float3 xx = x * x;
	return xx*xx*x;
}

float4 pow5(float4 x)
{
	float4 xx = x * x;
	return xx*xx*x;
}

float3 ComputeHalfVector(float3 v, float3 l)
{
	return normalize(v + l);
}

float4 MulQuaternion(float4 q1, float4 q2)
{
	float4 rs = (float4)0;
	rs.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
	rs.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);
	return rs;
}

float3 MulQuaternionVector(float4 q, float3 v)
{
	float3 t = 2.0 * cross(q.xyz, v);
	return v + q.w * t + cross(q.xyz, t);
}

float3 TransformPosition(float3 v, float4 q, float3 t)
{
	return MulQuaternionVector(q, v) + t;
}


float remap(float n, float min, float max, float dmin, float dmax)
{
	return (n - min) / (max - min) * (dmax - dmin) + dmin;
}
float2 remap(float2 n, float min, float max, float dmin, float dmax)
{
	return (n - min) / (max - min) * (dmax - dmin) + dmin;
}
float3 remap(float3 n, float min, float max, float dmin, float dmax)
{
	return (n - min) / (max - min) * (dmax - dmin) + dmin;
}
float4 remap(float4 n, float min, float max, float dmin, float dmax)
{
	return (n - min) / (max - min) * (dmax - dmin) + dmin;
}

float remapSimpleTo01(float n)
{
	return (n + 1) * 0.5;
}
float2 remapSimpleTo01(float2 n)
{
	return (n + 1) * 0.5;
}
float3 remapSimpleTo01(float3 n)
{
	return (n + 1) * 0.5;
}
float4 remapSimpleTo01(float4 n)
{
	return (n + 1) * 0.5;
}

float remapSimpleFrom01(float n)
{
	return n * 2 - 1;
}
float2 remapSimpleFrom01(float2 n)
{
	return n * 2 - 1;
}
float3 remapSimpleFrom01(float3 n)
{
	return n * 2 - 1;
}
float4 remapSimpleFrom01(float4 n)
{
	return n * 2 - 1;
}

#endif