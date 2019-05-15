#include "../Shared/pch.h"
#include "FrameResource.h"
#include "GraphicsDefine.h"


FrameResource::FrameResource()
{
}


FrameResource::~FrameResource()
{
}

void FrameResource::Initialize(UINT objectCount)
{
	ASSERT_HR(D3DDevice::pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdAlloc.GetAddressOf())
	));
	ObjectConstantBuffer.Initialize(objectCount, true);
}
