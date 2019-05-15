#pragma once
#include <d3dcompiler.h>
#include "GraphicsDefine.h"
namespace D3DGraphics
{
	static ComPtr<ID3DBlob>	CompileShader(
		wchar_t* fileName,
		D3D_SHADER_MACRO* marco,
		char* entryPoint,
		char* target,
		UINT flag = 0
		)
	{
		UINT shaderFlag = 0;
#if _DEBUG || DEBUG
		shaderFlag = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif
		ComPtr<ID3DBlob> shaderCode;
		ComPtr<ID3DBlob> errorCode;
		
		D3DCompileFromFile(
			fileName,
			marco,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint,
			target,
			shaderFlag,
			flag,
			shaderCode.GetAddressOf(),
			errorCode.GetAddressOf()
		);

		if (errorCode)
		{
			OutputDebugStringA((char*)errorCode->GetBufferPointer());
			ASSERT_HR(-1);
		}
		return shaderCode;
	}

	static ComPtr<ID3D12Resource> CreateDefaultBuffer(
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploader
	)
	{
		ComPtr<ID3D12Resource> buffer;

		ASSERT_HR(D3DDevice::pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(buffer.GetAddressOf())
		));

		ASSERT_HR(D3DDevice::pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploader.GetAddressOf())
		));

		D3D12_SUBRESOURCE_DATA data = {};
		data.pData = initData;
		data.RowPitch = byteSize;
		data.SlicePitch = data.RowPitch;

		D3DDevice::pCmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				buffer.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		);

		UpdateSubresources<1>(D3DDevice::pCmdList, buffer.Get(), uploader.Get(), 0, 0, 1, &data);
		D3DDevice::pCmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				buffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ
			)
		);

		return buffer;
	}
}