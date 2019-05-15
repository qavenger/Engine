#include "../Shared/pch.h"
#include "Camera.h"
#include "ComponentPool.h"
namespace EntityComponent
{
	DEFINE_COMPONENT_TYPE_ID(Camera);
	Camera* Camera::main = nullptr;
	void Camera::Initialize()
	{
		IComponent::Initialize();
	}
	void Camera::Initialize(float FOV, float width, float height, float nearZ, float farZ)
	{
		if (main == nullptr) main = this;
		m_width = width;
		m_height = height;
		m_aspectRatio = width / height;
		m_nearZ = nearZ;
		m_farZ = farZ;
		SetFOV(FOV);
	}
	void Camera::OnResize(UINT width, UINT height)
	{
		m_width = (float)width;
		m_height = (float)height;
		m_aspectRatio = m_width / m_height;
		BuildProjectionMatrix();
	}
	void Camera::Tick()
	{
		Transform* transform = m_gameObject->GetTransform();
		XMStoreFloat3(&m_frustum.Origin, transform->GetWorldPosition());
		XMStoreFloat4(&m_frustum.Orientation, transform->GetWorldRotation());
	}
}
