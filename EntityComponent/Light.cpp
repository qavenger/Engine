#include "../Shared/pch.h"
#include "Light.h"
#include "../D3DGraphics/GraphicsComponent.h"
#include "ComponentPool.h"
#include "Entity.h"
#include "Transform.h"
namespace EntityComponent
{
	DEFINE_COMPONENT_TYPE_ID(Light);
	LightInfo Light::Assign()const
	{
		LightInfo light;

		Transform& transform = *m_gameObject->GetTransform();
		XMStoreFloat4(&light.PosRadius, transform.GetWorldPosition());
		light.PosRadius.w = radius;

		light.ColorIntensity = { color.x, color.y, color.z, intensity };

		light.Falloff.x = falloffInner;
		light.Falloff.y = falloffOutter;
		light.Falloff.z = falloffNear;
		light.Falloff.w = falloffFar;
		XMStoreFloat3(&light.Direction, transform.Forward());

		light.type = lightType;

		return light;
	}
}