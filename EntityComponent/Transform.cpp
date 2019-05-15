#include "../Shared/pch.h"
#include "Transform.h"
#include "ComponentPool.h"
#include "Entity.h"
#include "../GameApp/MathDefine.h"
namespace EntityComponent
{
	DEFINE_COMPONENT_TYPE_ID(Transform);
	void Transform::Initialize()
	{
		IComponent::Initialize();
		m_parent = nullptr;
		m_NeedUpdate = false;
		m_children.clear();
		world = local = TransformData();
	}

	XMVECTOR Transform::Forward()const
	{
		XMVECTOR forward = XMLoadFloat4(&world.rotation);
		forward = XMVector3Rotate(XMVectorSet(0,0,1,0), forward);
		return forward;
	}

	void Transform::SetForward(XMVECTOR forward)
	{
		forward = XMVector3Normalize(forward);
		XMVECTOR right, up;
		XMVECTOR worldUP = G_UP;
		XMVECTOR cmp = XMVectorSet(1,1,1,1) - XMVector3Dot(forward, worldUP);
		if (XMVector3LessOrEqual(cmp, G_EPISILON))
		{
			up = XMVector3Normalize(XMVector3Cross(forward, G_RIGHT));
			right = XMVector3Cross(up, forward);
		}
		else
		{
			right = XMVector3Normalize(XMVector3Cross(worldUP, forward));
			up = XMVector3Cross(forward, right);
		}
		XMMATRIX M(right, up, forward, XMQuaternionIdentity());
		XMStoreFloat4(&world.rotation, XMQuaternionRotationMatrix(M));
	}

	void Transform::SetRight(XMVECTOR right)
	{
		right = XMVector3Normalize(right);
		XMVECTOR forward, up;
		XMVECTOR worldUP = G_UP;
		XMVECTOR cmp = XMVectorSet(1, 1, 1, 1) - XMVector3Dot(right, worldUP);
		if (XMVector3LessOrEqual(cmp, G_EPISILON))
		{
			up = XMVector3Normalize(XMVector3Cross(G_FORWARD, right));
			forward = XMVector3Cross(right, up);
		}
		else
		{
			forward = XMVector3Normalize(XMVector3Cross(right, worldUP));
			up = XMVector3Cross(forward, right);
		}
		XMMATRIX M(right, up, forward, XMQuaternionIdentity());
		XMStoreFloat4(&world.rotation, XMQuaternionRotationMatrix(M));
	}

	void Transform::SetUp(XMVECTOR up)
	{
		up = XMVector3Normalize(up);
		XMVECTOR right, forward;
		XMVECTOR worldForward = G_FORWARD;
		XMVECTOR cmp = XMVectorSet(1, 1, 1, 1) - XMVector3Dot(up, G_FORWARD);
		if (XMVector3LessOrEqual(cmp, G_EPISILON))
		{
			forward = XMVector3Normalize(XMVector3Cross(G_RIGHT, up));
			right = XMVector3Cross(up, forward);
		}
		else
		{
			right = XMVector3Normalize(XMVector3Cross(up, worldForward));
			forward = XMVector3Cross(right, up);
		}
		XMMATRIX M(right, up, forward, XMQuaternionIdentity());
		XMStoreFloat4(&world.rotation, XMQuaternionRotationMatrix(M));
	}

	void Transform::SetForward(const float x, const float y, const float z)
	{
		SetForward(XMVectorSet(x, y, z, 0));
	}
	void Transform::SetForward(const float3& forward)
	{
		SetForward(XMLoadFloat3(&forward));
	}

	void Transform::SetUp(const float x, const float y, const float z)
	{
		SetUp(XMVectorSet(x, y, z, 0));
	}
	void Transform::SetUp(const float3& up)
	{
		SetUp(XMLoadFloat3(&up));
	}

	void Transform::SetRight(const float x, const float y, const float z)
	{
		SetRight(XMVectorSet(x, y, z, 0));
	}
	void Transform::SetRight(const float3& right)
	{
		SetRight(XMLoadFloat3(&right));
	}

	XMVECTOR Transform::Up()const
	{
		XMVECTOR up = XMLoadFloat4(&world.rotation);
		up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), up);
		return up;
	}
	XMVECTOR Transform::Right()const
	{
		XMVECTOR right = XMLoadFloat4(&world.rotation);
		right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), right);
		return right;
	}
	XMMATRIX Transform::GetMatrix()const
	{
		return XMMatrixAffineTransformation(XMLoadFloat3(&world.scale), XMQuaternionIdentity(), XMLoadFloat4(&world.rotation), XMLoadFloat3(&world.position));
	}

	XMMATRIX Transform::GetInverseMatrix()const
	{
		XMVECTOR inv_pos = -XMLoadFloat3(&world.position);
		XMVECTOR inv_rot = XMLoadFloat4(&world.rotation);
		XMVECTOR inv_scl = XMLoadFloat3(&world.scale);
		XMVECTOR one = XMVectorSet(1, 1, 1, 1);
		inv_rot = XMQuaternionConjugate(inv_rot);
		inv_pos = XMVector3Rotate(inv_pos, inv_rot);
		inv_scl = one / inv_scl;

		return XMMatrixAffineTransformation(inv_scl, XMQuaternionIdentity(), inv_rot, inv_pos);
	}

	void Transform::Rotate(float pitch, float yaw, float roll)
	{
		Rotate(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
	}

	void Transform::RotateLocal(float pitch, float yaw, float roll)
	{
		RotateLocal(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
	}

	void Transform::Translate(float x, float y, float z)
	{
		Translate(XMVectorSet(x, y, z, 0));
	}

	void Transform::Translate(const XMVECTOR& offset)
	{
		XMVECTOR pos = XMLoadFloat3(&world.position);
		pos += offset;
		XMStoreFloat3(&world.position, pos);
	}

	void Transform::Rotate(const XMVECTOR& rot)
	{
		XMVECTOR rs_w = XMLoadFloat4(&world.rotation);
		rs_w = XMQuaternionNormalize(XMQuaternionMultiply(rot, rs_w));
		XMStoreFloat4(&world.rotation, rs_w);
	}

	void Transform::RotateLocal(const XMVECTOR& rot)
	{
		XMVECTOR rs_w = XMLoadFloat4(&world.rotation);
		rs_w = XMQuaternionNormalize(XMQuaternionMultiply(rs_w, rot));
		XMStoreFloat4(&world.rotation, rs_w);
	}
}