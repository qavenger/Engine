#pragma once
#include "../Shared/pch.h"
namespace EntityComponent
{
	typedef unsigned UINT;
	class Entity;
	class IComponent
	{
	public:
		IComponent() : m_bPooled(0), m_bStatic(0), m_bAlive(0), m_bEnable(0){};
		//called when the component is allocated
		virtual void Initialize() { m_bAlive = 1; m_bEnable = 1; };
		virtual void Shutdown() { m_bAlive = 0; SetEnable(0); };
		//called after the component is allocated
		virtual void Start() {}
		virtual void OnEnable() {}
		virtual void OnShutdown() {}
		virtual void OnDisable() {}
		virtual UINT GetTypeID()const = 0;
		static char* GetComponentTypeName() { return "IComponent"; };
		//called every visual frame
		virtual void Tick() {};
		//called every fixed amount of time
		virtual void Update() {};
		virtual ~IComponent() {}
		bool IsAlive()const { return m_bAlive; }
		bool IsEnable()const { return m_bEnable; }
		bool IsStatic()const { return m_bStatic; }
		void SetStatic(bool bStatic) { m_bStatic = bStatic; }
		Entity*	GetEntity()const { return m_gameObject; }
		void SetEnable(bool enabled) { m_bEnable = enabled; if (enabled)OnEnable(); else OnDisable(); }

		void SetPoolIndex(UINT idx) { if (!m_bPooled) m_poolIndex = idx; m_bPooled = 1; }
		UINT PoolIndex()const { return m_poolIndex; }
	protected:
		Entity*						m_gameObject;
		UINT						m_poolIndex;
	private:
		friend class ComponentPool;
		friend class Entity;
		void Attach(Entity* entity, bool enable = true) { m_gameObject = entity; m_bAlive = true; m_bEnable = enable; }
		void Detach() { m_gameObject = nullptr; m_bAlive = false; m_bEnable = false; };
	private:
		//Attached
		unsigned	m_bAlive : 1;
		//Enabled
		unsigned	m_bEnable : 1;
		//SetIndex
		unsigned	m_bPooled : 1;
		//Batch-Instance
		unsigned	m_bStatic : 1;
	};
}