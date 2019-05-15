#include "pch.h"
#include "EntityPool.h"
#include "Entity.h"

namespace EntityComponent {
	EntityPool::EntityPool()
	{
	}


	EntityPool::~EntityPool()
	{
	}
	void EntityPool::Initialize(UINT maxSize, UINT minIndex)
	{
		m_entities = new Entity[maxSize];
		m_numEntities = maxSize;
		m_nextChunk = nullptr;
		m_freeIndex.resize(maxSize);
		for (UINT i = 0; i < maxSize; ++i)
		{
			m_freeIndex[i] = i + minIndex;
			m_entities[i].SetPoolIndex(i + minIndex);
		}
	}
	void EntityPool::Shutdown()
	{
	}
	void EntityPool::AddChunk(UINT maxSize, UINT minIndex)
	{
		if (!m_nextChunk)
		{
			m_nextChunk = new EntityPool();
			if(m_nextChunk)
				m_nextChunk->Initialize(maxSize, minIndex);
		}
		else
		{
			m_nextChunk->AddChunk(maxSize, minIndex + m_numEntities);
		}
	}
	Entity * EntityPool::GetFreeEntity()
	{
		if (m_freeIndex.size() > 0)
		{
			UINT i = m_freeIndex.back();
			m_freeIndex.pop_back();
			return &m_entities[i];
		}
		else if(m_nextChunk)
		{
			return m_nextChunk->GetFreeEntity();
		}
		return nullptr;
	}
}