#pragma once
#include "IComponent.h"
#include "ComponentDefine.h"
class Material;
namespace EntityComponent
{
	BEGIN_DECLARE_COMPONENT(Renderable)
	public:
		Material*				material = nullptr;
		DirectX::BoundingOrientedBox		boundingBox;
		DirectX::BoundingSphere			boundingSphere;
	END_DECLARE_COMPONENT
}