#pragma once
class IComponent;
namespace EntityComponent
{

	class PoolBase
	{
	public:
		virtual void Initialize(UINT maxSize, bool bAutoTick, UINT minIndex = 0) = 0;
		virtual IComponent* GetObjects()const = 0;
		virtual void Shutdown() = 0;
		virtual IComponent* GetFreeObject() = 0;
		virtual void Tick() = 0;
		bool IsAutoTick()const { return m_bAutoTick; }
		UINT GetSize()const { return m_numObjects; }
		bool HasFree()const { return m_freeIndex.size(); }
		void ReturnToPool(UINT index);
		virtual PoolBase* AddChunk(UINT, UINT) = 0;
	protected:
		UINT				m_baseIndex;
		UINT				m_numObjects;
		std::vector<UINT>	m_freeIndex;
		PoolBase*			m_nextChunk;
		bool				m_bAutoTick;
	};
}
