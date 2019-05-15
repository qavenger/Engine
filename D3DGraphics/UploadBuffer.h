#pragma once
#include "../Shared/pch.h"
#include "GraphicsDefine.h"
template<typename T>
class UploadBuffer
{
public:
	UploadBuffer() {};
	~UploadBuffer() { if (m_pUploadBuffer) m_pUploadBuffer->Unmap(0, nullptr); m_data = nullptr; }
	void Initialize(UINT elementCount, bool isConstantBuffer)
	{
		m_bConstantBuffer = isConstantBuffer;
		m_elementByteSize = isConstantBuffer ? CB_SIZE(sizeof T) : sizeof T;
		ASSERT_HR(D3DDevice::pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize*elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pUploadBuffer.GetAddressOf())
		));

		ASSERT_HR(m_pUploadBuffer->Map(0, nullptr, (void**)&m_data));
	}
	ID3D12Resource*	Resource()const { return m_pUploadBuffer.Get(); }
	void CopyData(int elementIndex, const T* data)
	{
		memcpy(m_data + elementIndex * m_elementByteSize, data, sizeof T);
	}

	UINT Size()const { return m_elementByteSize / sizeof T; }
private:
	ComPtr<ID3D12Resource>		m_pUploadBuffer;
	UINT						m_elementByteSize;
	BYTE*						m_data = nullptr;
	bool						m_bConstantBuffer;
};

