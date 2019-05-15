#include "../Shared/pch.h"
#include "ShadowMap.h"
#include "D3DDevice.h"
#include "GraphicsDefine.h"

void ShadowMap::Initialize(UINT width, UINT height)
{
	m_uWidth = width;
	m_uHeight = height;
	m_viewPort = { 0,0,(float)width, (float)height, 0, 1 };
	m_rect = { 0, 0, (int)width, (int)height };
	BuildResource();
}

void ShadowMap::OnResize(UINT width, UINT height)
{
	if (width != m_uWidth || height != m_uHeight)
	{
		m_uWidth = width;
		m_uHeight = height;
		m_pShadowMap.Reset();
		m_viewPort = { 0,0,(float)width, (float)height, 0, 1 };
		m_rect = { 0, 0, (int)width, (int)height };
		BuildResource();
		BuildDescriptor();
	}
	
}

void ShadowMap::BuildDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE hSRVGPU, CD3DX12_CPU_DESCRIPTOR_HANDLE hSRVCPU, CD3DX12_CPU_DESCRIPTOR_HANDLE hDSVCPU)
{
	m_hSRVGpu = hSRVGPU;
	m_hSRVCpu = hSRVCPU;
	m_hDSVCpu = hDSVCPU;
	BuildDescriptor();
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = m_uWidth;
	desc.Height = m_uHeight;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = m_format;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clr = {};
	clr.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clr.DepthStencil.Depth = 1;
	clr.DepthStencil.Stencil = 0;
	ASSERT_HR(D3DDevice::pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		&clr,
		IID_PPV_ARGS(m_pShadowMap.GetAddressOf())
		)
	);
}

void ShadowMap::BuildDescriptor()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	D3DDevice::pDevice->CreateShaderResourceView(m_pShadowMap.Get(), &srvDesc, m_hSRVCpu);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3DDevice::pDevice->CreateDepthStencilView(m_pShadowMap.Get(), &dsvDesc, m_hDSVCpu);
}
