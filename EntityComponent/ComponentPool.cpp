#include "../Shared/pch.h"
#include "ComponentPool.h"
#include "IComponent.h"
#include "PoolBase.h"
#include "PoolObject.h"
#include "Entity.h"
#include "Camera.h"
#include "Transform.h"
#include "StaticMesh.h"
#include "Light.h"
#include "CameraController.h"

#define INITIALIZE_POOL(TYPE, AUTOUPDATE, SIZE) \
{PoolObject<TYPE>* pool = new PoolObject<TYPE>; pool->Initialize(SIZE, AUTOUPDATE); \
m_pools[TYPE::GetComponentType()] = (PoolBase*)pool;}

namespace EntityComponent {
	std::unordered_map<UINT, PoolBase*> ComponentPool::m_pools;

	UINT ComponentPool::m_typeCount;
	void ComponentPool::Initialize()
	{
		INITIALIZE_POOL(Entity, false, 1024)
		INITIALIZE_POOL(Transform, false, 1024)
		INITIALIZE_POOL(StaticMesh, false, 1024)
		INITIALIZE_POOL(Light, false, 1024)
		INITIALIZE_POOL(CameraController, true, 8)
		INITIALIZE_POOL(Camera, true, 8)
	}
	void ComponentPool::Tick()
	{
		for (auto itr = m_pools.begin(); itr != m_pools.end(); ++itr)
		{
			if (itr->second->IsAutoTick())
			{
				itr->second->Tick();
			}
		}
	}
	IComponent* ComponentPool::FindComponentsOfType(UINT typeID)
	{
		auto itr = m_pools.find(typeID);
		if (itr == m_pools.end())return nullptr;
		return itr->second->GetObjects();
	}
	IComponent * ComponentPool::GetFreeOfType(UINT typeID)
	{
		auto itr = m_pools.find(typeID);
		if (itr == m_pools.end()) return nullptr;
		return itr->second->GetFreeObject();
	}

	UINT ComponentPool::GetPoolSizeOfType(UINT typeID)
	{
		auto itr = m_pools.find(typeID);
		if (itr == m_pools.end()) return 0;
		return m_pools[typeID]->GetSize();
	}

	Entity * ComponentPool::SpawnEntity(float3 position, float4 rotation, Transform * parent)
	{
		Entity* entity = GetFreeOfType<Entity>();
		Transform* trans = entity->AddComponent<Transform>();
		entity->Initialize();
		trans->SetWorldPosition(position);
		trans->SetWorldRotation(rotation);
		trans->SetParent(parent);
		return entity;
	}
	void ComponentPool::Despawn(IComponent * component)
	{
		m_pools[component->GetTypeID()]->ReturnToPool(component->m_poolIndex);
		component->Shutdown();
	}

	Entity * ComponentPool::SpawnEntity()
	{
		Entity* entity = GetFreeOfType<Entity>();
		entity->AddComponent<Transform>();
		entity->Initialize();
		return entity;
	}
}