#pragma once
#include "../Shared/pch.h"
#include "PoolBase.h"
#include "IComponent.h"
namespace EntityComponent
{
	template<typename T>
	class PoolObject : public PoolBase
	{
	public:
		void Initialize(UINT maxSize, bool bAutoUpdate, UINT minIndex = 0) override;
		IComponent* GetObjects()const override;
		void Shutdown() override;
		IComponent* GetFreeObject() override;
		void Tick()override;
	protected:
		T*			m_objects;
		PoolBase* AddChunk(UINT maxSize, UINT minIndex = 0)override;
	};


	template<typename T>
	inline void PoolObject<T>::Initialize(UINT maxSize, bool bAutoTick, UINT minIndex)
	{
		m_bAutoTick = bAutoTick;
		maxSize = std::max<UINT>(maxSize, 1);
		m_numObjects = maxSize;
		m_objects = new T[maxSize];
		m_nextChunk = nullptr;
		m_freeIndex.resize(maxSize);
		m_baseIndex = minIndex;
		UINT idx;
		for (UINT i = 0; i < maxSize; ++i)
		{
			idx = minIndex + (maxSize - i - 1);
			m_freeIndex[i] = idx;
			m_objects[i].SetPoolIndex(i + minIndex);
		}
	}

	template<typename T>
	inline IComponent * PoolObject<T>::GetObjects() const
	{
		return m_objects;
	}

	template<typename T>
	inline void PoolObject<T>::Shutdown()
	{
		delete[]m_objects;
		if (m_nextChunk)
		{
			m_nextChunk->Shutdown();
		}
		delete m_nextChunk;
	}

	template<typename T>
	inline IComponent * PoolObject<T>::GetFreeObject()
	{
		IComponent* comp = nullptr;
		if (m_freeIndex.size() > 0)
		{
			UINT idx = m_freeIndex.back();
			m_freeIndex.pop_back();
			comp = &m_objects[idx];
		}
		else if (m_nextChunk)
		{
			comp = m_nextChunk->GetFreeObject();
		}
		
		if (!comp)
		{
			PoolBase* pool = AddChunk(m_numObjects);
			if (pool) comp = pool->GetFreeObject();
		}

		return comp;
	}

	template<typename T>
	inline void PoolObject<T>::Tick()
	{
		for (UINT i = 0; i < m_numObjects; ++i)
		{
			if(m_objects[i].IsAlive() && m_objects[i].IsEnable())
				m_objects[i].Tick();
		}
		if (m_nextChunk)
		{
			m_nextChunk->Tick();
		}
	}

	template<typename T>
	inline PoolBase * PoolObject<T>::AddChunk(UINT maxSize, UINT minIndex)
	{
		if (!m_nextChunk)
		{
			m_nextChunk = new PoolObject<T>;
			if (m_nextChunk) {
				m_nextChunk->Initialize(maxSize, minIndex);
			}
			return m_nextChunk;
		}
		else
		{
			return m_nextChunk->AddChunk(maxSize, minIndex + m_numObjects);
		}
	}

}