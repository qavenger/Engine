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

*/

VS_OUTPUT main( VertexBase input_base, VertexAddition input_data )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 worldPos = mul(float4(input_base.pos, 1), gWorld);
	float4 shadowPos = mul(worldPos, gShadowTransform);
	output.shadowPos = shadowPos.xyz / shadowPos.w;

	float3x3 world3x3 = (float3x3)gWorld;
	output.worldpos = worldPos.xyz;
	output.viewDir = normalize(gInvView[3].xyz - worldPos.xyz);
	output.pos = mul(worldPos , gViewProj);
	output.uv = input_base.uv;
	output.color = input_data.color;
	output.normal = mul(input_data.normal, world3x3);
	output.tangent = mul(input_data.tangent, world3x3);
	output.bitangent = cross(output.normal, output.tangent);
	return output;
}