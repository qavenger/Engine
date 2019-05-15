#pragma once
#include "D3DGraphics.h"
#include "FrameResource.h"
#include "ShadowMap.h"
#include "GraphicsComponent.h"
namespace D3DGraphics
{
#define NUM_FRAME_RESOURCE 3
#define MAX_OBJ 1024
	class Graphics : public D3DGraphics
	{
	public:
		Graphics();
		~Graphics();
		void Initialize(Window* pWnd) override;
		void PostInitialize()override;
		void OnResize()override;
		void Render() override;
		void Tick() override;
	private:
		void CreateDescriptorHeaps() override;
		void CreateCBDescriptorHeaps();
		void CreateCBV();
		void CreateShadersAndInputLayout();
		void CreateRootSignature();
		void CreatePSOs();
		void BuildPrimitive();
		void UploadFrameConstant();
		void UploadObjectConstant();
		void UploadLightConstant();
		void UploadShadowConstant();
		void RenderBegin(RenderQueueType pass);
		void CreateAssets();
	private:
		void BuildingRenderQueue();
	private:
		void ShadowPass();
	private:
		FrameResource					m_FrameResource[NUM_FRAME_RESOURCE];
		FrameResource*					m_pCurrentFrameResource;
		UINT							m_uCurrentFrameResourceIndex = 0;
		ObjectConstant					m_ObjectConstant[MAX_OBJ];
		UploadBuffer<PassConstant>		m_PassConstantBuffer;
		UploadBuffer<LightConstant>		m_LightConstantBuffer;
		PassConstant					m_PassConstant;
		PassConstant					m_ShadowPassConstant;
		ComPtr<ID3D12Resource>			m_pRTVBuffer[2];
		D3D12_CPU_DESCRIPTOR_HANDLE		m_hRTVCpu[2];
		D3D12_GPU_DESCRIPTOR_HANDLE		m_hRTVGpu[2];
		LightConstant					m_lightConstant;
		//ComPtr<ID3DBlob>				m_pShaderCodes[SHADER_NUM_TYPE];
		std::unordered_map<std::wstring, ComPtr<ID3DBlob>> m_pShaderCodes;
		std::unordered_map<std::wstring, ComPtr<ID3D12PipelineState>> m_pPSOs;
		RenderQueue						m_renderQueue;
		//ComPtr<ID3D12PipelineState>		m_pPSOs[PSO_NUM_TYPE];
		ComPtr<ID3D12DescriptorHeap>	m_pCBVHeap;
		ComPtr<ID3D12DescriptorHeap>	m_pSRVHeap;
		ComPtr<ID3D12RootSignature>		m_pRootSignature;
		INT								m_uFrameCBOffset;
		ShadowMap						m_shadowMap;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout[VERTEX_NUM_TYPE];
		bool							m_bWireframe = false;
	private:
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
	};
}