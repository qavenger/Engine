#pragma once
#include "IComponent.h"
#include "ComponentDefine.h"
#include "../D3DGraphics/GraphicsDefine.h"
namespace EntityComponent
{
BEGIN_DECLARE_COMPONENT(Transform)
public:
	struct TransformData
	{
		TransformData() :position({ 0,0,0 }), scale({ 1, 1, 1 }), rotation({ 0, 0, 0, 1 }) {}
		float3 position;
		float3 scale;
		float4 rotation;
	};

	void Initialize() override;
	void SetWorldPosition(const float3& pos) { world.position = pos; }
	void SetWorldPosition(const float x, const float y, const float z) { world.position = { x,y,z }; }
	void SetWorldPosition(const XMVECTOR& pos) { XMStoreFloat3(&world.position, pos); }

	void SetWorldRotation(const float4& quaternion) { world.rotation = quaternion; }
	void SetWorldRotation(const XMVECTOR& quaternion) { XMStoreFloat4(&world.rotation, quaternion); }

	void SetWorldScale(const float scale) { world.scale = { scale, scale, scale }; }
	void SetWorldScale(const float x, const float y, const float z) { world.scale = { x,y,z }; }
	void SetWorldScale(const XMVECTOR& scale) { XMStoreFloat3(&world.scale, scale); }

	XMVECTOR GetWorldPosition()const { return XMLoadFloat3(&world.position); }
	XMVECTOR GetWorldRotation()const { return XMLoadFloat4(&world.rotation); }
	XMVECTOR GetWorldScale() const { return XMLoadFloat3(&world.scale); }
	XMVECTOR GetLocalPosition()const { return XMLoadFloat3(&local.position); }
	XMVECTOR GetLocalRotation()const { return XMLoadFloat4(&local.rotation); }
	XMVECTOR GetLocalScale() const { return XMLoadFloat3(&local.scale); }
	float3	WorldPosition()const { return world.position; }
	float3	WorldScale()const { return world.scale; }
	float4	WorldRotation()const { return world.rotation; }

	void SetForward(const float x, const float y, const float z);
	void SetRight(const float x, const float y, const float z);
	void SetUp(const float x, const float y, const float z);

	void SetForward(const float3&);
	void SetRight(const float3&);
	void SetUp(const float3&);

	void SetForward(XMVECTOR);
	void SetRight(XMVECTOR);
	void SetUp(XMVECTOR);
	void Rotate(float pitch, float yaw, float roll);
	void RotateLocal(float pitch, float yaw, float roll);
	void Translate(float x, float y, float z);

	XMMATRIX GetMatrix()const;
	XMMATRIX GetInverseMatrix() const;
	void SetParent(Transform* parent) { m_parent = parent; }
public:
	XMVECTOR Forward()const;
	XMVECTOR Up()const;
	XMVECTOR Right()const;

	inline void Translate(const XMVECTOR& offset);
	inline void Rotate(const XMVECTOR& rot); 
	inline void RotateLocal(const XMVECTOR& rot);
private:
	TransformData	world;
	TransformData	local;
	Transform*	m_parent;
	std::list<Transform*>	m_children;
	bool	m_NeedUpdate;
END_DECLARE_COMPONENT
}