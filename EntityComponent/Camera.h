#pragma once
#include "IComponent.h"

#include "ComponentDefine.h"
#include "Entity.h"
#include "Transform.h"
#include "../GameApp/MathDefine.h"
namespace EntityComponent
{
BEGIN_DECLARE_COMPONENT(Camera)
public:
	void Initialize()override;
	void Tick()override;
	static Camera* const Main(){ return main; }
	void SetFOV(float FOVinDegree) { m_fov = FOVinDegree * 0.5f * G_DEG2RAD; BuildProjectionMatrix(); }
	void Initialize(float FOVinDegree, float width, float height, float nearZ, float farZ);
	void OnResize(UINT width, UINT height);
	XMMATRIX GetViewMatrix()const { return m_gameObject->GetTransform()->GetInverseMatrix(); }
	XMMATRIX GetViewMatrixInverse()const { return m_gameObject->GetTransform()->GetMatrix(); }
	XMMATRIX GetViewProjectionMatrix()const { return GetViewMatrix() * XMLoadFloat4x4(&m_proj); }
	XMMATRIX GetViewProjectionInverseMatrix()const { return XMLoadFloat4x4(&m_proj_inverse) * m_gameObject->GetTransform()->GetMatrix();}
	XMMATRIX GetProjectionMatrix()const { return XMLoadFloat4x4(&m_proj); }
	XMMATRIX GetInverseProjectionMatrix()const { return XMLoadFloat4x4(&m_proj_inverse); }
	BoundingFrustum const& GetViewFrustum() { return m_frustum; }
private:
	void BuildProjectionMatrix() { 
		XMMATRIX proj = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ); 
		XMStoreFloat4x4(&m_proj, proj);
		XMStoreFloat4x4(&m_proj_inverse, XMMatrixInverse(nullptr, proj)); 
		m_frustum.CreateFromMatrix(m_frustum, proj); 
	}
private:
	static Camera* main;
	BoundingFrustum m_frustum;
	float4x4	m_proj;
	float4x4	m_proj_inverse;
	float m_width;
	float m_height;
	float m_fov;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;
END_DECLARE_COMPONENT
}
