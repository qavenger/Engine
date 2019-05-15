#include "../../D3DGraphics/GraphicsDefine.h"
#include "ShaderCommon.hlsli"
/*
	
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
float4 pos			SEMANTIC(SV_POSITION)
float2 uv			SEMANTIC(TEXCOORD0)
float3 worldpos		SEMANTIC(POSITION)
float4 color		SEMANTIC(COLOR)
float3 normal		SEMANTIC(NORMAL)
float3 tangent		SEMANTIC(TANGENT)
float3 bitangent	SEMANTIC(TEXCOORD1)
};

*/

VS_OUTPUT_SIMPLE main( VertexBase input )
{
	VS_OUTPUT_SIMPLE output = (VS_OUTPUT_SIMPLE)0;
#if DEPTH
	float4 pos = mul(float4(input.pos, 1), gWorld);
	output.pos = mul(pos, gViewProj);
#endif

#if POST_PROCESS
	output.pos = float4(input.pos, 1);
#endif
	output.uv = input.uv;
	return output;
}