#pragma once
#include <dxgi.h>
#include <d3dcompiler.h>
class Window;
namespace D3DGraphics
{
	class D3DGraphics
	{
	public:
		D3DGraphics();
		virtual ~D3DGraphics();
		virtual void Initialize(Window* pWnd) = 0;
		virtual void PostInitialize() = 0;
		virtual void OnResize() = 0;
		virtual void Render() = 0;
		virtual void Tick() = 0;
		bool CheckDevice()const { return m_pDevice.Get() != nullptr; }
	private:

		D3DGraphics(const D3DGraphics&) = delete;
		D3DGraphics& operator=(const D3DGraphics&) = delete;

		void LogAdapter();
		void CheckFeatureSupport();
		void CreateDeviceAndFence();
		void CreateCommandQueues();
		void CreateCommandObjects();
		void CreateSwapChain();
		virtual void CreateDescriptorHeaps();
	protected:
		void FlushCommandQueue();
		ID3D12Resource*		CurrentBackBuffer()const;
		D3D12_CPU_DESCRIPTOR_HANDLE	CurrentBackBufferView()const;
		D3D12_CPU_DESCRIPTOR_HANDLE	CurrentDepthStencilView()const;

	protected:
		static const UINT							s_uNumBackBuffer = 2;
		int											m_currentBackBuffer = 0;
		ComPtr<IDXGIFactory1>						m_pFactory;
		ComPtr<ID3D12Device>						m_pDevice;
		ComPtr<IDXGISwapChain>						m_pSwapChain;
		ComPtr<ID3D12CommandQueue>					m_pGraphicsQueue;
		ComPtr<ID3D12CommandQueue>					m_pComputeQueue;
		ComPtr<ID3D12CommandQueue>					m_pCopyQueue;
		ComPtr<ID3D12GraphicsCommandList>			m_pGraphicsList;
		ComPtr<ID3D12CommandAllocator>				m_pGraphicsAllocator;
		ComPtr<ID3D12Fence>							m_pFence;
		ComPtr<ID3D12Resource>						m_pBackBuffer[s_uNumBackBuffer];
		ComPtr<ID3D12Resource>						m_pDepthBuffer;
		ComPtr<ID3D12DescriptorHeap>				m_pRTVHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pDSVHeap;
		UINT64										m_uFenceValue;
		D3D12_VIEWPORT								m_viewport;
		D3D12_RECT									m_rect;
		DXGI_FORMAT									m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		DXGI_FORMAT									m_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT									m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		std::vector<IDXGIAdapter*>					m_pAdapters;
		std::vector<DXGI_MODE_DESC>					m_displayMode;
		IDXGIAdapter*								m_pSelectedAdapter;
		D3D12_FEATURE_DATA_D3D12_OPTIONS			m_featureOption;
		D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT	m_featureGPU;
		UINT										m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		Window*										m_pWnd;
		
	};

}