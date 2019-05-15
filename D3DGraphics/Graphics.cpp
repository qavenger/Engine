#include "../Shared/pch.h"
#include "../Window/Window.h"
#include "GraphicsDefine.h"
#include "Graphics.h"
#include "../Input/Input.h"
#include "../Shared/Timer.h"
#include "../EntityComponent/StaticMesh.h"
#include "../EntityComponent/ComponentPool.h"
#include "Utility.h"
#include "PrimitiveGeometry.h"
#include "GraphicsComponent.h"
#include "ResourcePool.h"
#include "../EntityComponent/Light.h"
#include "../EntityComponent/Camera.h"

D3DGraphics::Graphics::Graphics()
{
}

D3DGraphics::Graphics::~Graphics()
{
}

void D3DGraphics::Graphics::Initialize(Window * pWnd)
{
	D3DGraphics::Initialize(pWnd);

	ASSERT_HR(m_pGraphicsList->Reset(m_pGraphicsAllocator.Get(), nullptr));
	for (int i = 0; i < 3; ++i)
	{
		m_FrameResource[i].Initialize(MAX_OBJ);
	}

	m_shadowMap.Initialize(2048, 2048);
	
	m_PassConstantBuffer.Initialize(2, true);
	m_LightConstantBuffer.Initialize(1, true);
	CreateCBDescriptorHeaps();
	CreateCBV();
	CreateShadersAndInputLayout();
	CreateRootSignature();
	CreatePSOs();
	
	m_pCurrentFrameResource = &m_FrameResource[m_uCurrentFrameResourceIndex];

	BuildPrimitive();
	m_renderQueue.Initialize();
}

void D3DGraphics::Graphics::PostInitialize()
{
	ASSERT_HR(m_pGraphicsList->Close());
	ID3D12CommandList* list[] = { m_pGraphicsList.Get() };
	m_pGraphicsQueue->ExecuteCommandLists(_countof(list), list);
	FlushCommandQueue();

	CreateAssets();
}

void D3DGraphics::Graphics::OnResize()
{
	assert(m_pDevice && m_pSwapChain && m_pGraphicsAllocator);
	FlushCommandQueue();

	ASSERT_HR(m_pGraphicsList->Reset(m_pGraphicsAllocator.Get(), nullptr));

	D3DGraphics::OnResize();

	D3D12_RESOURCE_DESC backDesc = m_pBackBuffer[0]->GetDesc();
	D3D12_CLEAR_VALUE clr_val = {};
	clr_val.Format = backDesc.Format;
	clr_val.Color[3] = 1;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = backDesc.Format;

	for (int i = 0; i < 2; ++i)
	{
		m_pRTVBuffer[i].Reset();
		ASSERT_HR(m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&backDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clr_val,
			//nullptr,
			IID_PPV_ARGS(m_pRTVBuffer[i].GetAddressOf())
		));
		m_hRTVCpu[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), s_uNumBackBuffer + i, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
		m_pDevice->CreateRenderTargetView(m_pRTVBuffer[i].Get(), &rtvDesc, m_hRTVCpu[i]);

	}

	if (m_pSRVHeap)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		auto hRTV_SRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pSRVHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < 2; ++i)
		{
			m_pDevice->CreateShaderResourceView(m_pRTVBuffer[i].Get(), &srvDesc, hRTV_SRV);
			m_hRTVGpu[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pSRVHeap->GetGPUDescriptorHandleForHeapStart(), i, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
			hRTV_SRV.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
		}
		
	}

	ASSERT_HR(m_pGraphicsList->Close());

	ID3D12CommandList* list[] = { m_pGraphicsList.Get() };
	m_pGraphicsQueue->ExecuteCommandLists(_countof(list), list);
	FlushCommandQueue();

	m_PassConstant.gScreenSize.x = (float)m_pWnd->Width;
	m_PassConstant.gScreenSize.y = (float)m_pWnd->Height;
	m_PassConstant.gScreenSize.z = 1 / m_PassConstant.gScreenSize.x;
	m_PassConstant.gScreenSize.w = 1 / m_PassConstant.gScreenSize.y;


}
void D3DGraphics::Graphics::Render()
{
	ID3D12CommandAllocator* alloc = m_pCurrentFrameResource->CmdAlloc.Get();
	ASSERT_HR(alloc->Reset() );
	ASSERT_HR(m_pGraphicsList->Reset(alloc, m_pPSOs[L"shadow"].Get()));

	m_pGraphicsList->SetGraphicsRootSignature(m_pRootSignature.Get());

	ShadowPass();
	static float color[4] = { 0.00f, 0.00f, 0.00f, 1 };
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = CurrentDepthStencilView();
	m_pGraphicsList->ClearRenderTargetView(m_hRTVCpu[0], color, 0, nullptr);
	m_pGraphicsList->ClearRenderTargetView(m_hRTVCpu[1], color, 0, nullptr);
	m_pGraphicsList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1, 0, 0, nullptr);
	m_pGraphicsList->RSSetViewports(1, &m_viewport);
	m_pGraphicsList->RSSetScissorRects(1, &m_rect);

	m_pGraphicsList->SetGraphicsRootConstantBufferView(1, m_PassConstantBuffer.Resource()->GetGPUVirtualAddress());
	m_pGraphicsList->SetGraphicsRootConstantBufferView(4, m_LightConstantBuffer.Resource()->GetGPUVirtualAddress());

	m_pGraphicsList->SetPipelineState(m_pPSOs[L"pre_depth"].Get());
	m_pGraphicsList->OMSetRenderTargets(0, nullptr, false, &dsv);
	RenderBegin(RenderQueueType::OPAQUE);

	ID3D12DescriptorHeap* descHeap[] = { m_pSRVHeap.Get() };
	m_pGraphicsList->SetDescriptorHeaps(_countof(descHeap), descHeap);
	auto srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pSRVHeap->GetGPUDescriptorHandleForHeapStart(),2, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
	m_pGraphicsList->SetGraphicsRootDescriptorTable(2, m_shadowMap.SRV());
	m_pGraphicsList->SetGraphicsRootDescriptorTable(3, srvHandle);

	if (m_bWireframe)
	{
		m_pGraphicsList->SetPipelineState(m_pPSOs[L"opaque_wireframe"].Get());
	}
	else
	{
		m_pGraphicsList->SetPipelineState(m_pPSOs[L"opaque"].Get());
	}
	m_pGraphicsList->OMSetRenderTargets(2, m_hRTVCpu, true, &dsv);
	RenderBegin(RenderQueueType::OPAQUE);


	m_pGraphicsList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	m_pGraphicsList->SetPipelineState(m_pPSOs[L"post_process"].Get());
	m_pGraphicsList->OMSetRenderTargets(1, &backBufferView, true, nullptr);
	auto* mesh = MeshPool::meshes[L"Primitive"];
	auto& sub = mesh->DrawArgs[L"postQuad"];
	m_pGraphicsList->IASetVertexBuffers(0, 1, mesh->VertexBufferView);
	m_pGraphicsList->IASetIndexBuffer(&mesh->IndexBufferView);

	m_pGraphicsList->SetGraphicsRootDescriptorTable(3, m_pSRVHeap->GetGPUDescriptorHandleForHeapStart());
	m_pGraphicsList->DrawIndexedInstanced(mesh->DrawArgs[L"postQuad"].IndexCount, 1,sub.StartIndexLocation ,sub.BaseVertexLocation, 0);

	m_pGraphicsList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

	ASSERT_HR(m_pGraphicsList->Close());
	ID3D12CommandList* list[] = { m_pGraphicsList.Get() };
	m_pGraphicsQueue->ExecuteCommandLists(_countof(list), list);
	ASSERT_HR(m_pSwapChain->Present(0, 0));
	m_currentBackBuffer = (m_currentBackBuffer + 1) & 1;

	m_pCurrentFrameResource->FenceValue = ++m_uFenceValue;
	m_pGraphicsQueue->Signal(m_pFence.Get(), m_uFenceValue);
}

void D3DGraphics::Graphics::Tick()
{
	if (Input::IsKeyPressed(Input::KeyCode::F3))
	{
		m_bWireframe = !m_bWireframe;
	}

	++m_uCurrentFrameResourceIndex;
	m_uCurrentFrameResourceIndex *= m_uCurrentFrameResourceIndex < NUM_FRAME_RESOURCE;
	m_pCurrentFrameResource = &m_FrameResource[m_uCurrentFrameResourceIndex];
	UINT64 fenceValue = m_pCurrentFrameResource->FenceValue;
	if (fenceValue != 0 && m_pFence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		ASSERT_HR(m_pFence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	BuildingRenderQueue();
	UploadObjectConstant();
	UploadLightConstant();
	UploadShadowConstant();
	UploadFrameConstant();
}

void D3DGraphics::Graphics::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = s_uNumBackBuffer + 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pRTVHeap.GetAddressOf())));

	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pDSVHeap.GetAddressOf())));
}

void D3DGraphics::Graphics::CreateCBDescriptorHeaps()
{
	UINT numDescriptors = NUM_FRAME_RESOURCE * MAX_OBJ + 2 + 1;
	m_uFrameCBOffset = NUM_FRAME_RESOURCE * MAX_OBJ;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pCBVHeap.GetAddressOf())));
}

void D3DGraphics::Graphics::CreateCBV()
{
	UINT objCBByteSize = sizeof ObjectConstant;

	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pCBVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int frameIdx = 0; frameIdx < NUM_FRAME_RESOURCE; ++frameIdx)
	{
		auto cbAddress = m_FrameResource[frameIdx].ObjectConstantBuffer.Resource()->GetGPUVirtualAddress();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		for (UINT i = 0; i < MAX_OBJ; ++i)
		{
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_pDevice->CreateConstantBufferView(&cbvDesc, handle);
			cbAddress += objCBByteSize;
			handle.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
		}
	}

	UINT frameCBByteSize = sizeof PassConstant;

	auto cbAddress = m_PassConstantBuffer.Resource()->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = frameCBByteSize;
	m_pDevice->CreateConstantBufferView(&cbvDesc, handle);

	cbAddress += frameCBByteSize;
	handle.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);

	cbvDesc.BufferLocation = cbAddress;
	m_pDevice->CreateConstantBufferView(&cbvDesc, handle);

	handle.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);

	UINT lightCBByteSize = sizeof LightConstant;
	cbAddress = m_LightConstantBuffer.Resource()->GetGPUVirtualAddress();
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = lightCBByteSize;
	m_pDevice->CreateConstantBufferView(&cbvDesc, handle);
}

void D3DGraphics::Graphics::CreateShadersAndInputLayout()
{
	D3D_SHADER_MACRO marco[] =
	{
		{ "ALPHA_TEST", "1" },
		{ nullptr, nullptr }
	};
	D3D_SHADER_MACRO marco_postprocess[] =
	{
		{ "DEPTH", "0" },
		{ "POST_PROCESS", "1" },
		{ nullptr, nullptr }
	};
	D3D_SHADER_MACRO marco_depth[] =
	{
		{"DEPTH", "1"},
		{"POST_PROCESS", "0" },
		{nullptr, nullptr}
	};
	m_pShaderCodes[L"Standard_VS"] = CompileShader(L"..\\GameApp\\HLSL\\standard_vs.hlsl", nullptr, "main", "vs_5_1");
	m_pShaderCodes[L"Standard_PS"] = CompileShader(L"..\\GameApp\\HLSL\\opaque_ps.hlsl", nullptr, "main", "ps_5_1");
	m_pShaderCodes[L"PostProcess_VS"] = CompileShader(L"..\\GameApp\\HLSL\\depth_vs.hlsl", marco_postprocess, "main", "vs_5_1");
	m_pShaderCodes[L"PostProcess_PS"] = CompileShader(L"..\\GameApp\\HLSL\\postprocess_ps.hlsl", nullptr, "main", "ps_5_1");
	m_pShaderCodes[L"Depth_VS"] = CompileShader(L"..\\GameApp\\HLSL\\depth_vs.hlsl", marco_depth, "main", "vs_5_1");
	m_pShaderCodes[L"Depth_PS"] = CompileShader(L"..\\GameApp\\HLSL\\depth_ps.hlsl", nullptr, "main", "ps_5_1");
	m_pShaderCodes[L"Depth_Alpha_PS"] = CompileShader(L"..\\GameApp\\HLSL\\depth_ps.hlsl", marco, "main", "ps_5_1");

	m_inputLayout[VERTEX_SIMPLE] =
	{
		{ "POSITION" , 0, DXGI_FORMAT_R32G32B32_FLOAT		, 0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT			, 0,	12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_inputLayout[VERTEX_STANDARD] =
	{
		{"POSITION"	, 0, DXGI_FORMAT_R32G32B32_FLOAT		, 0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT			, 0,	12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT		, 1,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT		, 1,	16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TANGENT"	, 0, DXGI_FORMAT_R32G32B32_FLOAT		, 1,	28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void D3DGraphics::Graphics::CreateRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParam[5];

	CD3DX12_DESCRIPTOR_RANGE texTable[2];
	texTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	texTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);
	
	slotRootParam[0].InitAsConstantBufferView(0); // obj cb
	slotRootParam[1].InitAsConstantBufferView(1); // pass cb
	slotRootParam[2].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL); // shadow depth
	slotRootParam[3].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL); // textures
	slotRootParam[4].InitAsConstantBufferView(2); // light cb
	
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[] =
	{
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
		CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
		CD3DX12_STATIC_SAMPLER_DESC(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
		CD3DX12_STATIC_SAMPLER_DESC(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
		CD3DX12_STATIC_SAMPLER_DESC(4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
		CD3DX12_STATIC_SAMPLER_DESC(5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
		CD3DX12_STATIC_SAMPLER_DESC(6, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
	};
	
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParam, 7, samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		ASSERT_HR(-1);
	}
	
	ASSERT_HR(
		m_pDevice->CreateRootSignature(
			0, 
			serializedRootSig->GetBufferPointer(), 
			serializedRootSig->GetBufferSize(), 
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf()
			)
		)
	);
}

void D3DGraphics::Graphics::CreatePSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.NumElements = (UINT)m_inputLayout[VERTEX_STANDARD].size();
	psoDesc.InputLayout.pInputElementDescs = m_inputLayout[VERTEX_STANDARD].data();
	psoDesc.pRootSignature = m_pRootSignature.Get();

	ID3DBlob* shaderCode = m_pShaderCodes[L"Standard_VS"].Get();

	psoDesc.VS.pShaderBytecode = shaderCode->GetBufferPointer();
	psoDesc.VS.BytecodeLength = shaderCode->GetBufferSize();

	shaderCode = m_pShaderCodes[L"Standard_PS"].Get();

	psoDesc.PS.pShaderBytecode = shaderCode->GetBufferPointer();
	psoDesc.PS.BytecodeLength = shaderCode->GetBufferSize();

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 2;
	psoDesc.RTVFormats[0] = m_swapChainFormat;
	psoDesc.RTVFormats[1] = m_swapChainFormat;
	//psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	
	psoDesc.DSVFormat = m_depthStencilFormat;
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSOs[L"opaque"].GetAddressOf())));

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC finalDesc = psoDesc;
	finalDesc.RTVFormats[0] = m_backBufferFormat;
	finalDesc.InputLayout.NumElements = (UINT)m_inputLayout[VERTEX_SIMPLE].size();
	finalDesc.InputLayout.pInputElementDescs = m_inputLayout[VERTEX_SIMPLE].data();
	finalDesc.DepthStencilState.DepthEnable = false;
	shaderCode = m_pShaderCodes[L"PostProcess_VS"].Get();
	finalDesc.VS = { shaderCode->GetBufferPointer() , shaderCode->GetBufferSize() };
	shaderCode = m_pShaderCodes[L"PostProcess_PS"].Get();
	finalDesc.PS = { shaderCode->GetBufferPointer() , shaderCode->GetBufferSize() };
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&finalDesc, IID_PPV_ARGS(m_pPSOs[L"post_process"].GetAddressOf())));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowDesc = psoDesc;
	shadowDesc.InputLayout.NumElements = (UINT)m_inputLayout[VERTEX_SIMPLE].size();
	shadowDesc.InputLayout.pInputElementDescs = m_inputLayout[VERTEX_SIMPLE].data();
	shadowDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	shadowDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	shadowDesc.NumRenderTargets = 0;
	shaderCode = m_pShaderCodes[L"Depth_VS"].Get();
	shadowDesc.VS.pShaderBytecode = shaderCode->GetBufferPointer();
	shadowDesc.VS.BytecodeLength = shaderCode->GetBufferSize();



	D3D12_GRAPHICS_PIPELINE_STATE_DESC preDepth = shadowDesc;
	preDepth.PS = { nullptr,0 };
	preDepth.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&preDepth, IID_PPV_ARGS(m_pPSOs[L"pre_depth"].GetAddressOf())));

	shaderCode = m_pShaderCodes[L"Depth_PS"].Get();
	shadowDesc.PS.pShaderBytecode = shaderCode->GetBufferPointer();
	shadowDesc.PS.BytecodeLength = shaderCode->GetBufferSize();
	shadowDesc.RasterizerState.SlopeScaledDepthBias = 0.7f;
	shadowDesc.RasterizerState.DepthBias = 32500;

	
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&shadowDesc, IID_PPV_ARGS(m_pPSOs[L"shadow"].GetAddressOf())));

	shaderCode = m_pShaderCodes[L"Depth_Alpha_PS"].Get();
	shadowDesc.PS.pShaderBytecode = shaderCode->GetBufferPointer();
	shadowDesc.PS.BytecodeLength = shaderCode->GetBufferSize();

	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&shadowDesc, IID_PPV_ARGS(m_pPSOs[L"shadow_alpha"].GetAddressOf())));

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.NumRenderTargets = 2;
	psoDesc.RTVFormats[0] = m_swapChainFormat;
	psoDesc.RTVFormats[1] = m_swapChainFormat;
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSOs[L"opaque_doubeSide"].GetAddressOf())));
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//psoDesc.RasterizerState.AntialiasedLineEnable = true;
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSOs[L"opaque_wireframe"].GetAddressOf())));

	//psoDesc.RasterizerState.AntialiasedLineEnable = false;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
	blendDesc.BlendEnable = true;
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentDesc = psoDesc;
	transparentDesc.BlendState.RenderTarget[0] = blendDesc;
	transparentDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&transparentDesc, IID_PPV_ARGS(m_pPSOs[L"transparent"].GetAddressOf())));

	CD3DX12_BLEND_DESC blendDescNoColorWrite(D3D12_DEFAULT);
	blendDescNoColorWrite.RenderTarget->RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthDesc.StencilEnable = true;
	depthDesc.StencilReadMask = 0xff;
	depthDesc.StencilWriteMask = 0xff;
	depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	
	depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	//psoDesc.NumRenderTargets = 1;
	//psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC markDesc = psoDesc;
	markDesc.BlendState = blendDescNoColorWrite;
	markDesc.DepthStencilState = depthDesc;
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&markDesc, IID_PPV_ARGS(m_pPSOs[L"stencil_mask"].GetAddressOf())));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC stencilDrawDesc = psoDesc;
	depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	stencilDrawDesc.DepthStencilState = depthDesc;
	//stencilDrawDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	//stencilDrawDesc.RasterizerState.FrontCounterClockwise = true;
	ASSERT_HR(m_pDevice->CreateGraphicsPipelineState(&stencilDrawDesc, IID_PPV_ARGS(m_pPSOs[L"stencil_draw"].GetAddressOf())));
}

void D3DGraphics::Graphics::BuildPrimitive()
{
	PrimitiveGeometry primitiveGen;
	PrimitiveGeometry::MeshData plane = primitiveGen.CreatePlane(1, 1, 1, 1);
	PrimitiveGeometry::MeshData box = primitiveGen.CreateBox(1,1,1,0);
	PrimitiveGeometry::MeshData cylinder = primitiveGen.CreateCylinder(0.5f, 0.5f, 1, 18, 1);
	PrimitiveGeometry::MeshData sphere = primitiveGen.CreateSphere(0.5f, 16, 12);
	PrimitiveGeometry::MeshData geoSphere = primitiveGen.CreateGeoSphere(0.5, 2);
	PrimitiveGeometry::MeshData postQuad;
	postQuad.Vertex.resize(4);
	postQuad.Vertex[0].base.pos = { -1, -1, 0 };
	postQuad.Vertex[0].base.uv = { 0, 1 };
	postQuad.Vertex[1].base.pos = { 1, -1, 0 };
	postQuad.Vertex[1].base.uv = { 1, 1 };
	postQuad.Vertex[2].base.pos = { -1, 1, 0 };
	postQuad.Vertex[2].base.uv = { 0, 0 };
	postQuad.Vertex[3].base.pos = { 1, 1, 0 };
	postQuad.Vertex[3].base.uv = { 1, 0 };
	postQuad.Index32.resize(6);
	postQuad.Index32[0] = 0;
	postQuad.Index32[1] = 2;
	postQuad.Index32[2] = 1;
	postQuad.Index32[3] = 1;
	postQuad.Index32[4] = 2;
	postQuad.Index32[5] = 3;

	UINT planeVertexOffset = 0;
	UINT planeIndexOffset = 0;

	UINT boxVertexOffset = planeVertexOffset + (UINT)plane.Vertex.size();
	UINT boxIndexOffset = planeIndexOffset +(UINT)plane.Index32.size();;

	UINT cylinderVertexOffset = boxVertexOffset + (UINT)box.Vertex.size();
	UINT cylinderIndexOffset = boxIndexOffset + (UINT)box.Index32.size();

	UINT sphereVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertex.size();
	UINT sphereIndexOffset = cylinderIndexOffset + (UINT)cylinder.Index32.size();

	UINT geoSphereVertexOffset = sphereVertexOffset + (UINT)sphere.Vertex.size();
	UINT geoSphereIndexOffset = sphereIndexOffset + (UINT)sphere.Index32.size();

	UINT postQuadVertexOffset = geoSphereVertexOffset + (UINT)geoSphere.Vertex.size();
	UINT postQuadIndexOffset = geoSphereIndexOffset + (UINT)geoSphere.Index32.size();

	SubMesh planeSub = {};
	SubMesh boxSub = {};
	SubMesh cylinderSub = {};
	SubMesh sphereSub = {};
	SubMesh geosphereSub = {};
	SubMesh postQuadSub = {};

	planeSub.BaseVertexLocation = planeVertexOffset;
	planeSub.StartIndexLocation = planeIndexOffset;
	planeSub.IndexCount = (UINT)plane.Index32.size();
	planeSub.Bound.Extents = { 0.5f, 0.0001f, 0.5f };

	boxSub.BaseVertexLocation = boxVertexOffset;
	boxSub.StartIndexLocation = boxIndexOffset;
	boxSub.IndexCount = (UINT)box.Index32.size();
	boxSub.Bound.Extents = { 0.5f, 0.5f, 0.5f };

	cylinderSub.BaseVertexLocation = cylinderVertexOffset;
	cylinderSub.StartIndexLocation = cylinderIndexOffset;
	cylinderSub.IndexCount = (UINT)cylinder.Index32.size();
	cylinderSub.Bound.Extents = { 0.5f, 0.5f, 0.5f };

	sphereSub.BaseVertexLocation = sphereVertexOffset;
	sphereSub.StartIndexLocation = sphereIndexOffset;
	sphereSub.IndexCount = (UINT)sphere.Index32.size();
	sphereSub.Bound.Extents = { 0.5f, 0.5f, 0.5f };

	geosphereSub.BaseVertexLocation = geoSphereVertexOffset;
	geosphereSub.StartIndexLocation = geoSphereIndexOffset;
	geosphereSub.IndexCount = (UINT)geoSphere.Index32.size();
	geosphereSub.Bound.Extents = { 0.5f, 0.5f, 0.5f };

	postQuadSub.BaseVertexLocation = postQuadVertexOffset;
	postQuadSub.StartIndexLocation = postQuadIndexOffset;
	postQuadSub.IndexCount = (UINT)postQuad.Index32.size();

	size_t totalVertCount = plane.Vertex.size() + box.Vertex.size() + cylinder.Vertex.size() + sphere.Vertex.size() + geoSphere.Vertex.size() + postQuad.Vertex.size();

	std::vector<VertexBase> vb(totalVertCount);
	std::vector<VertexAddition> va(totalVertCount);
	std::vector<uint16_t> idx;
	idx.insert(idx.end(), plane.GetIndex16().begin(), plane.GetIndex16().end());
	idx.insert(idx.end(), box.GetIndex16().begin(), box.GetIndex16().end());
	idx.insert(idx.end(), cylinder.GetIndex16().begin(), cylinder.GetIndex16().end());
	idx.insert(idx.end(), sphere.GetIndex16().begin(), sphere.GetIndex16().end());
	idx.insert(idx.end(), geoSphere.GetIndex16().begin(), geoSphere.GetIndex16().end());
	idx.insert(idx.end(), postQuad.GetIndex16().begin(), postQuad.GetIndex16().end());
	UINT k = 0;
	size_t i;

	SubMesh allSub = {};
	allSub.IndexCount = (UINT)idx.size();
	allSub.Bound.Center = { 0,0,0 };
	allSub.Bound.Extents = { 0.5f,0.5f,0.5f };
	for (i = 0; i < plane.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(plane.Vertex[i].base);
		va[k] = std::move(plane.Vertex[i].addition);
	}
	for (i = 0; i < box.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(box.Vertex[i].base);
		va[k] = std::move(box.Vertex[i].addition);
	}
	for (i = 0; i < cylinder.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(cylinder.Vertex[i].base);
		va[k] = std::move(cylinder.Vertex[i].addition);
	}
	for (i = 0; i < sphere.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(sphere.Vertex[i].base);
		va[k] = std::move(sphere.Vertex[i].addition);
	}
	for (i = 0; i < geoSphere.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(geoSphere.Vertex[i].base);
		va[k] = std::move(geoSphere.Vertex[i].addition);
	}
	for (i = 0; i < postQuad.Vertex.size(); ++i, ++k)
	{
		vb[k] = std::move(postQuad.Vertex[i].base);
		va[k] = std::move(postQuad.Vertex[i].addition);
	}
	const UINT vbByteSize = (UINT)(vb.size() * sizeof VertexBase);
	const UINT vaByteSize = (UINT)(va.size() * sizeof VertexAddition);
	const UINT iByteSize = (UINT)(idx.size() * sizeof uint16_t);

	Mesh* mesh = new Mesh;
	mesh->Name = L"Primitive";

	mesh->IndexCount = (UINT)idx.size();
	ASSERT_HR(D3DCreateBlob(vbByteSize + vaByteSize, mesh->VertexBufferCPU.GetAddressOf()));
	memcpy(mesh->VertexBufferCPU->GetBufferPointer(), vb.data(), vbByteSize);
	memcpy((BYTE*)mesh->VertexBufferCPU->GetBufferPointer() + vbByteSize, va.data(), vaByteSize);

	ASSERT_HR(D3DCreateBlob(iByteSize, mesh->IndexBufferCPU.GetAddressOf()));
	memcpy(mesh->IndexBufferCPU->GetBufferPointer(), idx.data(), iByteSize);

	mesh->VertexBuffer = CreateDefaultBuffer(mesh->VertexBufferCPU->GetBufferPointer(), mesh->VertexBufferCPU->GetBufferSize(), mesh->VertexUploader);
	
	mesh->IndexBuffer = CreateDefaultBuffer(mesh->IndexBufferCPU->GetBufferPointer(), mesh->IndexBufferCPU->GetBufferSize(), mesh->IndexUploader);

	mesh->VertexBufferView[0].BufferLocation = mesh->VertexBuffer->GetGPUVirtualAddress();
	mesh->VertexBufferView[0].SizeInBytes = vbByteSize;
	mesh->VertexBufferView[0].StrideInBytes= sizeof VertexBase;

	mesh->VertexBufferView[1].BufferLocation = mesh->VertexBufferView[0].BufferLocation + vbByteSize;
	mesh->VertexBufferView[1].SizeInBytes = vaByteSize;
	mesh->VertexBufferView[1].StrideInBytes = sizeof VertexAddition;

	mesh->VertexBufferView[2].BufferLocation = mesh->VertexBufferView[1].BufferLocation + vaByteSize;
	mesh->VertexBufferView[2].SizeInBytes = 0;
	mesh->VertexBufferView[2].StrideInBytes = sizeof VertexSkin;

	mesh->IndexBufferView.BufferLocation = mesh->IndexBuffer->GetGPUVirtualAddress();
	mesh->IndexBufferView.Format = mesh->IndexFormat;
	mesh->IndexBufferView.SizeInBytes = (UINT)mesh->IndexBufferCPU->GetBufferSize();
	mesh->DrawArgs[L"Plane"] = std::move(planeSub);
	mesh->DrawArgs[L"Box"] = std::move(boxSub);
	mesh->DrawArgs[L"Cylinder"] = std::move(cylinderSub);
	mesh->DrawArgs[L"Sphere"] = std::move(sphereSub);
	mesh->DrawArgs[L"GeoSphere"] = std::move(geosphereSub);
	mesh->DrawArgs[L"postQuad"] = std::move(postQuadSub);
	mesh->DrawArgs[L"All"] = std::move(allSub);
	MeshPool::meshes[mesh->Name] = std::move(mesh);
}

void D3DGraphics::Graphics::UploadFrameConstant()
{
	using namespace EntityComponent;
	const Camera* camera = Camera::Main();
	XMMATRIX vi = camera->GetViewMatrixInverse();
	XMMATRIX v = XMMatrixInverse(nullptr, vi);
	XMMATRIX p = camera->GetProjectionMatrix();
	XMMATRIX pi = camera->GetViewProjectionInverseMatrix();
	XMMATRIX vp = v * p;
	XMMATRIX vpi = pi * vi;

	XMStoreFloat4x4(&m_PassConstant.gView, v);
	XMStoreFloat4x4(&m_PassConstant.gInvView, vi);
	XMStoreFloat4x4(&m_PassConstant.gProj, p);
	XMStoreFloat4x4(&m_PassConstant.gInvProj, pi);
	XMStoreFloat4x4(&m_PassConstant.gViewProj, vp);
	XMStoreFloat4x4(&m_PassConstant.gInvViewProj, vpi);

	m_PassConstant.gTime.x = g_Timer.DeltaTime() * 0.1f;
	m_PassConstant.gTime.y = g_Timer.DeltaTime();
	m_PassConstant.gTime.z = g_Timer.DeltaTime() * 5;
	m_PassConstant.gTime.w = (float)g_Timer.Elapsed();
	m_PassConstantBuffer.CopyData(0, &m_PassConstant);
}

void D3DGraphics::Graphics::UploadObjectConstant()
{
	using namespace EntityComponent;
	StaticMesh* meshes = ComponentPool::FindComponentsOfType<StaticMesh>();
	UINT size = ComponentPool::GetPoolSizeOfType<StaticMesh>();
	
	for (UINT i = 0, j = 0; i < size; ++i)
	{
		StaticMesh& mesh = meshes[i];
		if (mesh.IsAlive() && mesh.IsEnable())
		{
			XMMATRIX W = mesh.GetEntity()->GetTransform()->GetMatrix();
			XMStoreFloat4x4(&m_ObjectConstant[j].gWorld, W);
			XMStoreFloat4x4(&m_ObjectConstant[j].gInverseWorld, XMMatrixInverse(nullptr, W));
			m_pCurrentFrameResource->ObjectConstantBuffer.CopyData(j, &m_ObjectConstant[j]);
			++j;
		}
	}
}

void D3DGraphics::Graphics::UploadLightConstant()
{
	using namespace EntityComponent;
	Light* lights = ComponentPool::FindComponentsOfType<Light>();
	UINT size = ComponentPool::GetPoolSizeOfType<Light>();

	UINT numLights = 0;
	for (UINT i = 0; i < size; ++i)
	{
		Light& light = lights[i];
		if (light.IsAlive() && light.IsEnable())
		{
			m_lightConstant.lights[numLights] = light.Assign();
			++numLights;
			if (numLights >= MAX_LIGHT_COUNT)break;
		}
	}
	m_lightConstant.gNumLights = numLights;
	m_LightConstantBuffer.CopyData(0, &m_lightConstant);
}

void D3DGraphics::Graphics::UploadShadowConstant()
{
	using namespace EntityComponent;
	Light* lights = ComponentPool::FindComponentsOfType<Light>();
	UINT size = ComponentPool::GetPoolSizeOfType<Light>();
	for (UINT i = 0, j = 0; i < size; ++i)
	{
		Light& light = lights[i];
		if (light.IsAlive() && light.IsEnable() && light.lightType == LIGHT_DIRECTIONAL)
		{
			Transform* transform = light.GetEntity()->GetTransform();
			XMVECTOR forward = transform->Forward() * -17.7f;
			XMMATRIX vi = transform->GetMatrix() * XMMatrixTranslationFromVector(forward);
			
			XMMATRIX v = XMMatrixInverse(nullptr, vi);

			XMMATRIX p = XMMatrixOrthographicLH(35.4f,35.4f, 0.0f, 35.4f);
			XMMATRIX pi = XMMatrixInverse(nullptr, p);
			XMMATRIX vp = v * p;
			XMMATRIX vpi = pi * vi;
			XMMATRIX T = 
			{	0.5f,  0.0f, 0, 0,
				0.0f, -0.5f, 0, 0,
				0.0f,  0.0f, 1, 0,
				0.5f,  0.5f, 0, 1 };
			XMMATRIX vpt = vp * T;
			XMStoreFloat4x4(&m_PassConstant.gShadowTransform, vpt);
			XMStoreFloat4x4(&m_ShadowPassConstant.gView, v);
			XMStoreFloat4x4(&m_ShadowPassConstant.gInvView, vi);
			XMStoreFloat4x4(&m_ShadowPassConstant.gProj, p);
			XMStoreFloat4x4(&m_ShadowPassConstant.gInvProj, pi);
			XMStoreFloat4x4(&m_ShadowPassConstant.gViewProj, vp);
			XMStoreFloat4x4(&m_ShadowPassConstant.gInvViewProj, vpi);
			m_ShadowPassConstant.gTime.x = g_Timer.DeltaTime() * 0.1f;
			m_ShadowPassConstant.gTime.y = g_Timer.DeltaTime();
			m_ShadowPassConstant.gTime.z = g_Timer.DeltaTime() * 5;
			m_ShadowPassConstant.gTime.w = (float)g_Timer.Elapsed();
			m_ShadowPassConstant.gScreenSize.x = (float)m_shadowMap.Width();
			m_ShadowPassConstant.gScreenSize.y = (float)m_shadowMap.Height();
			m_ShadowPassConstant.gScreenSize.x = 1.0f / m_shadowMap.Width();
			m_ShadowPassConstant.gScreenSize.y = 1.0f / m_shadowMap.Height();
			m_PassConstantBuffer.CopyData(1, &m_ShadowPassConstant);
			break;
		}
	}

	
}

void D3DGraphics::Graphics::RenderBegin(RenderQueueType pass)
{
	using namespace EntityComponent;
	/*StaticMesh* meshes = ComponentPool::FindComponentsOfType<StaticMesh>();
	UINT size = ComponentPool::GetPoolSizeOfType<StaticMesh>();*/

	auto cbAddress = m_pCurrentFrameResource->ObjectConstantBuffer.Resource()->GetGPUVirtualAddress();
	auto& queue = m_renderQueue.queue[pass];
	for (UINT i = 0; i < (UINT)queue.size(); ++i)
	{
		StaticMesh& mesh = *queue[i].mesh;
	
		m_pGraphicsList->IASetVertexBuffers(0, 2, mesh.mesh->VertexBufferView);
		m_pGraphicsList->IASetIndexBuffer(&mesh.mesh->IndexBufferView);
		m_pGraphicsList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		m_pGraphicsList->SetGraphicsRootConstantBufferView(0, cbAddress + queue[i].cbIdx * sizeof ObjectConstant);
		
		SubMesh* sub = mesh.subMesh;
		m_pGraphicsList->DrawIndexedInstanced(sub->IndexCount, 1, sub->StartIndexLocation, sub->BaseVertexLocation, 0);
		
	}
}

void D3DGraphics::Graphics::CreateAssets()
{
	UINT numTextures = TexturePool::GetNumTextures();
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 2 + numTextures + 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ASSERT_HR(m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pSRVHeap.GetAddressOf())));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_pSRVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < 2; ++i)
	{
		m_pDevice->CreateShaderResourceView(m_pRTVBuffer[i].Get(), &srvDesc, handle);
		m_hRTVGpu[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pSRVHeap->GetGPUDescriptorHandleForHeapStart(), i, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
		handle.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
	}

	Texture* texture = nullptr;
	for (UINT i = 0; i < numTextures; ++i)
	{
		texture = TexturePool::GetTexture(i);
		auto rcsDesc = texture->Resource->GetDesc();
		srvDesc.Format = rcsDesc.Format;
		switch (rcsDesc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MipLevels = rcsDesc.MipLevels;
			break;
		case D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (rcsDesc.DepthOrArraySize > 1)
			{
				if (rcsDesc.DepthOrArraySize == 6)
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				else
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			srvDesc.Texture2D.MipLevels = rcsDesc.MipLevels;
			break;
		case D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = rcsDesc.MipLevels;
			break;
		case D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			break;
		default:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;
			break;
		}

		m_pDevice->CreateShaderResourceView(texture->Resource.Get(),&srvDesc, handle);
		handle.Offset(m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCPUStart(m_pSRVHeap->GetCPUDescriptorHandleForHeapStart(), 2 + numTextures, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvGPUStart(m_pSRVHeap->GetGPUDescriptorHandleForHeapStart(), 2 + numTextures, m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCPUStart(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart(), 1 , m_uDescriptorSizes[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]);

	m_shadowMap.BuildDescriptor(srvGPUStart, srvCPUStart, dsvCPUStart);
	
}

void D3DGraphics::Graphics::BuildingRenderQueue()
{
	m_renderQueue.ClearQueue();
	using namespace EntityComponent;
	StaticMesh* meshes = ComponentPool::FindComponentsOfType<StaticMesh>();
	UINT size = ComponentPool::GetPoolSizeOfType<StaticMesh>();
	BoundingFrustum frustum = Camera::Main()->GetViewFrustum();
	BoundingFrustum	light;
	BoundingFrustum::CreateFromMatrix(light, XMLoadFloat4x4(&m_ShadowPassConstant.gProj));
	XMVECTOR s, t, r;
	XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m_ShadowPassConstant.gInvView));
	XMStoreFloat3(&light.Origin, t);
	XMStoreFloat4(&light.Orientation, r);
	RenderObject obj;
	UINT count = 0;
	for (UINT i = 0; i < size; ++i)
	{
		StaticMesh* mesh = &meshes[i];
		if (mesh->IsAlive() && mesh->IsEnable())
		{
			Transform* transform = mesh->GetEntity()->GetTransform();

			obj.mesh = mesh;
			obj.cbIdx = count++;
			if (light.Contains(mesh->boundingBox))
			{
				m_renderQueue.Add(RenderQueueType::SHADOW, obj);
			}

			if (frustum.Contains(mesh->boundingBox))
			{
				m_renderQueue.Add(RenderQueueType::OPAQUE, std::move(obj));
			}
			
		}
	}
}

void D3DGraphics::Graphics::ShadowPass()
{
	m_pGraphicsList->RSSetViewports(1, m_shadowMap.Viewport());
	m_pGraphicsList->RSSetScissorRects(1, m_shadowMap.Rect());

	m_pGraphicsList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap.Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	m_pGraphicsList->ClearDepthStencilView(m_shadowMap.DSV(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	m_pGraphicsList->OMSetRenderTargets(0, nullptr, false, &m_shadowMap.DSV());
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = m_PassConstantBuffer.Resource()->GetGPUVirtualAddress() + sizeof PassConstant;
	m_pGraphicsList->SetGraphicsRootConstantBufferView(1, passCBAddress);
	RenderBegin(RenderQueueType::SHADOW);
	m_pGraphicsList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap.Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

}
