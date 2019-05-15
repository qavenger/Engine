#pragma once
#include "IComponent.h"
#include "ComponentDefine.h"
#include "../D3DGraphics/GraphicsDefine.h"
namespace EntityComponent
{
BEGIN_DECLARE_COMPONENT(Light)
public:
	LightInfo Assign()const;

public:
	float3	color;
	float	intensity;
	float	falloffInner;
	float	falloffOutter;
	float	falloffNear;
	float	falloffFar;
	float	radius;
	uint	lightType;
END_DECLARE_COMPONENT
}