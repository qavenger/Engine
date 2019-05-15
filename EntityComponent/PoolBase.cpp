#include "../Shared/pch.h"
#include "PoolBase.h"

void EntityComponent::PoolBase::ReturnToPool(UINT index)
{
	if (index < m_numObjects + m_baseIndex)
		m_freeIndex.push_back(index);
	else if (m_nextChunk)
		m_nextChunk->ReturnToPool(index);
}
