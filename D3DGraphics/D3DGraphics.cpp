#include "../Shared/pch.h"
#include "../Window/Window.h"
#include "GraphicsDefine.h"
#include "D3DGraphics.h"
#include "D3DDevice.h"

D3DGraphics::D3DGraphics::D3DGraphics()
{
}

	
D3DGraphics::D3DGraphics::~D3DGraphics()
{
}

void D3DGraphics::D3DGraphics::Initialize(Window* pWnd)
{
//#if _DEBUG || DEBUG
		ComPtr<ID3D12Debug>		pDebug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(pDebug.GetAddressOf()))))
		{
			pDebug->EnableDebugLayer();
		}
//#endif 
	m_pWnd = pWnd;
	ASSERT_HR(CreateDXGIFactory1(IID_PPV_ARGS(m_pFactory.GetAddressOf())));
	LogAdapter();
	CreateDeviceAndFence();
	CheckFeatureSupport();
	CreateCommandQueues();
	CreateCommandObjects();
	CreateSwapChain();
	CreateDescriptorHeaps();

	OnResize();
}

void D3DGraphics::D3DGraphics::OnResize()
{

	int i;
	for (i = 0; i < s_uNumBackBuffer; ++i)
	{
		m_pBackBuffer[i].Reset();
	}
	m_pDepthBuffer.Reset();
	UINT width = m_pWnd->Width, height = m_pWnd->Height;
	ASSERT_HR(m_pSwapChain->ResizeBuffers(
		s_uNumBackBuffer,
		width,
		height,
		m_swapChainFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	));
	
	m_currentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_backBufferFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (i = 0; i < s_uNumBackBuffer; ++i)
	{
		ASSERT_HR(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pBackBuffer[i].GetAddressOf())));

		m_pDevice->CreateRenderTargetView(m_pBackBuffer[i].Get(), &rtvDesc, rtvHandle);
		rtvHandle.Offset(1, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
	}

	D3D12_RESOURCE_DESC dsDesc = {};
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsDesc.Height = height;
	dsDesc.Width = width;
	dsDesc.MipLevels = 1;
	dsDesc.SampleDesc.Count = 1;

	D3D12_CLEAR_VALUE colorVal;
	colorVal.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	colorVal.DepthStencil.Depth = 1;
	colorVal.DepthStencil.Stencil = 0;

	ASSERT_HR(m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&dsDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&colorVal,
		IID_PPV_ARGS(m_pDepthBuffer.GetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	m_pDevice->CreateDepthStencilView(
		m_pDepthBuffer.Get(), 
		&dsvDesc, 
		m_pDSVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	m_pGraphicsList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));


	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Height = (float)height;
	m_viewport.Width = (float)width;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_rect = {0, 0, (int)width, (int)height};
}
void D3DGraphics::D3DGraphics::LogAdapter()
{
	IDXGIAdapter* pAdapter;
	DXGI_ADAPTER_DESC adapterDesc;
	size_t maxMemory = 0;
	for (UINT i = 0; m_pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		pAdapter->GetDesc(&adapterDesc);
		if (adapterDesc.DedicatedVideoMemory > 0)
		{
			if (maxMemory < adapterDesc.DedicatedVideoMemory)
			{
				maxMemory = adapterDesc.DedicatedVideoMemory;
				m_pSelectedAdapter = pAdapter;
			}
			m_pAdapters.push_back(pAdapter);

		}
	}
	m_pAdapters.shrink_to_fit();
	assert(m_pAdapters.size() > 0);

	IDXGIOutput* pOutput;
	for (UINT numOutput = 0; m_pSelectedAdapter->EnumOutputs(numOutput, &pOutput) != DXGI_ERROR_NOT_FOUND; ++numOutput)
	{
		UINT count;
		pOutput->GetDisplayModeList(m_swapChainFormat, 0, &count, nullptr);
		m_displayMode.resize(count);
		pOutput->GetDisplayModeList(m_swapChainFormat, 0, &count, &m_displayMode[0]);
	}
}
void D3DGraphics::D3DGraphics::CheckFeatureSupport()
{
	m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_featureOption, sizeof m_featureOption);
	m_pDevice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &m_featureGPU, sizeof m_featureGPU);
}
void D3DGraphics::D3DGraphics::CreateDeviceAndFence()
{
	ASSERT_HR(
		D3D12CreateDevice(
			m_pSelectedAdapter, 
			D3D_FEATURE_LEVEL_11_0, 
			IID_PPV_ARGS(m_pDevice.GetAddressOf())
		)
	);
	ASSERT_HR(
		m_pDevice->CreateFence(
			0, 
			D3D12_FENCE_FLAG_NONE, 
			IID_PPV_ARGS(m_pFence.GetAddressOf())
		)
	);
	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_uDescriptorSizes[i] = m_pDevice->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);
	}
	D3DDevice::pDevice = m_pDevice.Get();
}
void D3DGraphics::D3DGraphics::CreateCommandQueues()
{
	D3D12_COMMAND_QUEUE_DESC cmdQDesc = {};
	cmdQDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ASSERT_HR(
		m_pDevice->CreateCommandQueue(
			&cmdQDesc, 
			IID_PPV_ARGS(m_pGraphicsQueue.GetAddressOf())
		)
	);
	cmdQDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ASSERT_HR(
		m_pDevice->CreateCommandQueue(
			&cmdQDesc,
			IID_PPV_ARGS(m_pComputeQueue.GetAddressOf())
		)
	);
	cmdQDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ASSERT_HR(
		m_pDevice->CreateCommandQueue(
			&cmdQDesc,
			IID_PPV_ARGS(m_pCopyQueue.GetAddressOf())
		)
	);
}

void D3DGraphics::D3DGraphics::CreateCommandObjects()
{
	ASSERT_HR(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pGraphicsAllocator.GetAddressOf())));
	ASSERT_HR(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pGraphicsAllocator.Get(), nullptr, IID_PPV_ARGS(m_pGraphicsList.GetAddressOf())));
	m_pGraphicsList->Close();
	D3DDevice::pCmdList = m_pGraphicsList.Get();
}

void D3DGraphics::D3DGraphics::CreateSwapChain()
{
	m_pSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferCount = s_uNumBackBuffer;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator= 1;
	scDesc.BufferDesc.Width = m_pWnd->Width;
	scDesc.BufferDesc.Height = m_pWnd->Height;
 	scDesc.SampleDesc.Count = 1;

	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.Windowed = m_pWnd->WindowMode();
	scDesc.OutputWindow = m_pWnd->Handle();
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ASSERT_HR(
		m_pFactory->CreateSwapChain(
			m_pGraphicsQueue.Get(),
			&scDesc,
			m_pSwapChain.GetAddressOf()
		)
	);
}

void D3DGraphics::D3DGraphics::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = s_uNumBackBuffer;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pRTVHeap.GetAddressOf())));

	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pDSVHeap.GetAddressOf())));
}

void D3DGraphics::D3DGraphics::FlushCommandQueue()
{
	++m_uFenceValue;
	ASSERT_HR(m_pGraphicsQueue->Signal(m_pFence.Get(), m_uFenceValue));
	if (m_pFence->GetCompletedValue() < m_uFenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		ASSERT_HR(m_pFence->SetEventOnCompletion(m_uFenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource * D3DGraphics::D3DGraphics::CurrentBackBuffer() const
{
	return m_pBackBuffer[m_currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DGraphics::D3DGraphics::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBuffer, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DGraphics::D3DGraphics::CurrentDepthStencilView() const
{
	return m_pDSVHeap->GetCPUDescriptorHandleForHeapStart();
}
