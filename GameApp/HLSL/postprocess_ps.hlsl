#include "../../D3DGraphics/GraphicsDefine.h"
#include "ShaderCommon.hlsli"

texture2D gTextures[2] : register(t1);

float4 main(VS_OUTPUT_SIMPLE input) : SV_TARGET
{
	/*float2 uv = input.uv;
	uv = remap(uv, 0, 1, -kPIDIV4, kPIDIV4);
	float r = atan2(uv.y, uv.x);
	r = frac(remap(-.031 * r + gTime[3] * 0.1, -1, 1, 0, kPI * 3)) ;
	r = 1 - pow(r, 64);*/
	float4 color = gTextures[0].Sample(gPointClamp, input.uv);
	//color *= color;
	//return color;
	float4 emissive = gTextures[1].Sample(gPointClamp, input.uv);
	//return emissive;
	/*float4 color1 = gCustomMap.Sample(gPointClamp, cos(input.pos.xy * 0.1)) * float4(1,0.4,0.7,1);
	color = lerp(color, color1 , 1 - r);*/
	return color + emissive;
}