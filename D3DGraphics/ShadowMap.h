#pragma once
class ShadowMap
{
public:
	void Initialize(UINT width, UINT height);
	void OnResize(UINT width, UINT height);
	void BuildDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE, CD3DX12_CPU_DESCRIPTOR_HANDLE, CD3DX12_CPU_DESCRIPTOR_HANDLE);
	ID3D12Resource*		Resource()const { return m_pShadowMap.Get(); }
	D3D12_VIEWPORT* Viewport() { return &m_viewPort; }
	D3D12_RECT*	Rect() { return &m_rect; }
	UINT Width()const { return m_uWidth; }
	UINT Height()const { return m_uHeight; }

	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV()const { return m_hSRVGpu; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE DSV()const { return m_hDSVCpu; }
private:
	void BuildResource();
	void BuildDescriptor();
private:
	CD3DX12_GPU_DESCRIPTOR_HANDLE	m_hSRVGpu;
	CD3DX12_CPU_DESCRIPTOR_HANDLE	m_hSRVCpu;
	CD3DX12_CPU_DESCRIPTOR_HANDLE	m_hDSVCpu;
	UINT							m_uWidth;
	UINT							m_uHeight;
	DXGI_FORMAT						m_format = DXGI_FORMAT_R24G8_TYPELESS;
	D3D12_VIEWPORT					m_viewPort;
	D3D12_RECT						m_rect;
	ComPtr<ID3D12Resource>			m_pShadowMap;
};

