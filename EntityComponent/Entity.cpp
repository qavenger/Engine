#include "../Shared/pch.h"
#include "Entity.h"
#include "ComponentPool.h"
#include "Transform.h"
namespace EntityComponent {
	DEFINE_COMPONENT_TYPE_ID(Entity);
	void Entity::Initialize()
	{
		IComponent::Initialize();
		m_gameObject = this;
		for (int i = 0; i < m_pComponents.size(); ++i)
		{
			m_pComponents[i]->Initialize();
		}
		m_transform = GetComponent<Transform>();
	}

	IComponent * Entity::AddComponent(UINT typeID)
	{
		IComponent* comp = ComponentPool::GetFreeOfType(typeID);
		if (comp)
		{
			comp->Initialize();
			comp->Attach(this);
			m_pComponents.push_back(comp);
		}
		return comp;
	}
	IComponent * Entity::GetComponent(UINT typeID) const
	{
		for (size_t i = 0; i < m_pComponents.size(); ++i)
		{
			if (m_pComponents[i]->GetTypeID() == typeID)
			{
				return m_pComponents[i];
			}
		}
		return nullptr;
	}
	Transform * Entity::GetTransform()const
	{
		return m_transform;
	}
	void Entity::Shutdown()
	{
		IComponent::Shutdown();
		for (int i = 0; i < m_pComponents.size(); ++i)
		{
			ComponentPool::Despawn(m_pComponents[i]);
		}
		m_pComponents.clear();
		m_transform = nullptr;
	}
}