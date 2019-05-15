#pragma once
#include "../D3DGraphics/GraphicsDefine.h"
namespace EntityComponent {
	typedef unsigned int UINT;
	class IComponent;
	class PoolBase;
	class Entity;
	class Transform;
	class ComponentPool
	{
	public:
		static void Initialize();
		static IComponent* FindComponentsOfType(UINT typeID);
		static IComponent* GetFreeOfType(UINT typeID);
		static UINT GetPoolSizeOfType(UINT typeID);
		static void Tick();
		static Entity*	SpawnEntity();
		static Entity*	SpawnEntity(float3 position, float4 rotation, Transform* parent = nullptr);
		static void Despawn(IComponent* component);
		template<typename T>
		static T* FindComponentsOfType();

		template<typename T>
		static T* GetFreeOfType();

		template<typename T>
		static UINT GetPoolSizeOfType();

		static UINT GetComponentTypeID() { return m_typeCount++; }
	private:
		static std::unordered_map<UINT, PoolBase*> m_pools;
		static UINT	m_typeCount;
	};

	template<typename T>
	inline T* ComponentPool::FindComponentsOfType()
	{
		return (T*)FindComponentsOfType(T::GetComponentType());
	}
	template<typename T>
	inline T * ComponentPool::GetFreeOfType()
	{
		return (T*)GetFreeOfType(T::GetComponentType());
	}
	template<typename T>
	inline UINT ComponentPool::GetPoolSizeOfType()
	{
		return GetPoolSizeOfType(T::GetComponentType());
	}

	static void Destroy(IComponent* component) { ComponentPool::Despawn(component); }
	static Entity* Spawn(float3 position = { 0,0,0 }, float4 rotation = { 0,0,0,1 }, Transform* parent = nullptr) { return ComponentPool::SpawnEntity(position, rotation, parent); }
}