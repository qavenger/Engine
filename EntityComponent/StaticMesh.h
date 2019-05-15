#pragma once
#include "IComponent.h"
#include "ComponentDefine.h"
#include "Renderable.h"
class Mesh;
struct SubMesh;
class Material;
namespace EntityComponent
{
BEGIN_DECLARE_COMPONENT_DERIVE(StaticMesh, Renderable)
public:
	Mesh*		mesh = nullptr;
	SubMesh*	subMesh = nullptr;
END_DECLARE_COMPONENT
}