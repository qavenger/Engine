#pragma once
#include "IComponent.h"

#include "ComponentDefine.h"
namespace EntityComponent {
	class Transform;

	BEGIN_DECLARE_COMPONENT(Entity)
	public:
		void Initialize()override;
		void Shutdown()override;
		IComponent* AddComponent(UINT typeID);
		template<typename T>
		T* AddComponent();

		IComponent* GetComponent(UINT typeID)const;
		template<typename T>
		T* GetComponent()const;
	public:
		Transform* GetTransform()const;
	private:
		Transform* m_transform = nullptr;
		std::vector<IComponent*>	m_pComponents;

		friend class ComponentPool;
	END_DECLARE_COMPONENT

	template<typename T>
	inline T * Entity::AddComponent()
	{
		return (T*)AddComponent(T::GetComponentType());
	}

	template<typename T>
	inline T * Entity::GetComponent() const
	{
		return (T*)GetComponent(T::GetComponentType());
	}

}