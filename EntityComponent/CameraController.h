#pragma once
#include "IComponent.h"
#include "ComponentDefine.h"
namespace EntityComponent
{
BEGIN_DECLARE_COMPONENT(CameraController)
public:
	void Tick()override;
	void Rotate(float pitch, float yaw, float roll);
private:
	void Forward(float speed);
	void Strafe(float speed);
	void Fly(float speed);
	void Yaw(float speed);
	void Pitch(float speed);
END_DECLARE_COMPONENT
}