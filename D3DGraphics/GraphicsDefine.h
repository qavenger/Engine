#ifndef _SHADERDEFINE_H
#define _SHADERDEFINE_H
#ifdef __cplusplus
	#include "D3DDevice.h"
	using namespace DirectX;
	typedef unsigned int	uint;
	typedef XMFLOAT2 float2;
	typedef XMFLOAT3 float3;
	typedef XMFLOAT4 float4;
	typedef XMFLOAT3X3 float3x3;
	typedef XMFLOAT4X4 float4x4;
	#define ASSERT_HR(hr) assert(SUCCEEDED(hr))
	#define _regAlign _declspec(align(256))
	#define SEMANTIC(s)	;
	#define	CB_START(cbName, reg)	struct _regAlign cbName{
	#define CB_END	};
	#define CB_SIZE(x) ((x + 255) & ~255)
	namespace D3DGraphics {
		enum PSOType
		{
			PSO_OPAQUE,
			PSO_OPAQUE_DOUBLE_SIDE,
			PSO_OPAQUE_WIREFRAME,
			PSO_TRANSPARENT,
			PSO_TRANSPARENT_WIREFRAME,
			PSO_STENCIL_MARK,
			PSO_STENCIL_DRAW,
			PSO_NUM_TYPE
		};
		enum ShaderType
		{
			SHADER_VS_STANDARD,
			SHADER_PS_OPAQUE,
			SHADER_VS_POSTPROCESS,
			SHADER_NUM_TYPE
		};
		enum VertexType
		{
			VERTEX_SIMPLE,
			VERTEX_STANDARD,
			VERTEX_SKINED,
			VERTEX_NUM_TYPE
		};

		enum LightType
		{
			LIGHT_DIRECTIONAL,
			LIGHT_POINT,
			LIGHT_SPOT,
			LIGHT_AREA,
			LIGHT_NUM_TYPE
		};

	}
#else
	#pragma pack_matrix(row_major)
	#define	SEMANTIC(s) : s;
	#define	CB_START(cbName, reg)	cbuffer cbName : register(reg){
	#define CB_END	}
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_AREA 3
#define LIGHT_NUM_TYPE 4
#endif
#define MAX_LIGHT_COUNT 32

CB_START(ObjectConstant, b0)
	float4x4	gWorld;
	float4x4	gInverseWorld;
CB_END

CB_START(PassConstant, b1)
	float4x4	gView;
	float4x4	gInvView;
	float4x4	gProj;
	float4x4	gInvProj;
	float4x4	gViewProj;
	float4x4	gInvViewProj;
	float4x4	gShadowTransform;
	float4		gTime;
	float4		gScreenSize;
CB_END

struct PS_OUTPUT
{
	float4 color		SEMANTIC(SV_TARGET0)
	float4 emissive		SEMANTIC(SV_TARGET1)
};

struct LightInfo
{
	float4 PosRadius;
	float4 ColorIntensity;
	float4 Falloff;
	float3 Direction;
	uint type;
};

CB_START(LightConstant, b2)
	LightInfo lights[MAX_LIGHT_COUNT];
	uint gNumLights;
	float3 gLightConstPading;
CB_END

struct VertexBase
{
	float3 pos		SEMANTIC(POSITION)
	float2 uv		SEMANTIC(TEXCOORD0)
};

struct VertexAddition
{
	float4 color	SEMANTIC(COLOR)
	float3 normal	SEMANTIC(NORMAL)
	float3 tangent  SEMANTIC(TANGENT)
};

struct VertexSkin
{

};

struct VS_OUTPUT
{
	float4 pos			SEMANTIC(SV_POSITION0)
	float2 uv			SEMANTIC(TEXCOORD0)
	float3 worldpos		SEMANTIC(POSITION0)
	float3 shadowPos	SEMANTIC(POSITION1)
	float3 viewDir		SEMANTIC(TEXCOORD1)
	float4 color		SEMANTIC(COLOR)
	float3 normal		SEMANTIC(NORMAL)
	float3 tangent		SEMANTIC(TANGENT0)
	float3 bitangent	SEMANTIC(TANGENT1)
};

struct VS_OUTPUT_SIMPLE
{
	float4 pos			SEMANTIC(SV_POSITION)
	float2 uv			SEMANTIC(TEXCOORD0)
};


typedef VS_OUTPUT PS_INPUT;
typedef VS_OUTPUT GS_INPUT;

#endif