#pragma once
#include "UploadBuffer.h"
class FrameResource
{
public:
	FrameResource();
	~FrameResource();
	void Initialize(UINT objectCount);
public:
	ComPtr<ID3D12CommandAllocator>		CmdAlloc;
	UploadBuffer<ObjectConstant>		ObjectConstantBuffer;
	UINT64								FenceValue = 0;
private:
	FrameResource(const FrameResource& rhs);
	FrameResource& operator=(const FrameResource& rhs);
};

