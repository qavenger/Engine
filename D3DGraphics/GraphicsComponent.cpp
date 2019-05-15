#include "../EntityComponent/Renderable.h"
#include "GraphicsComponent.h"
void RenderQueue::Initialize()
{
	queue.resize(NUM_RENDER_QUEUE_TYPE);

}

void RenderQueue::ClearQueue()
{
	for (int i = 0; i < (int)queue.size(); ++i)
	{
		queue[i].clear();
	}
}

void RenderQueue::Add(RenderQueueType type, RenderObject obj)
{
	queue[type].push_back(std::move(obj));
}
