#include "../../D3DGraphics/GraphicsDefine.h"
#include "ShaderCommon.hlsli"

Texture2D gTextures[2]			: register(t1);


void main(VS_OUTPUT_SIMPLE input)
{
#if ALPHA_TEST
	float4 base = gTextures[0].Sample(gAnisotropicWrap, input.uv);
	clip(base.a - 0.01);
#endif
}