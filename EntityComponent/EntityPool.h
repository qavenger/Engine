#pragma once

namespace EntityComponent {
	class Entity;
	class EntityPool
	{
	public:
		EntityPool();
		~EntityPool();
		void Initialize(UINT maxSize, UINT minIndex = 0);
		void Shutdown();
		void AddChunk(UINT maxSize, UINT minIndex = 0);
		Entity* GetFreeEntity();
	private:
		Entity*				m_entities;
		UINT				m_numEntities;
		std::vector<UINT>	m_freeIndex;
		EntityPool*			m_nextChunk;
	};
}

