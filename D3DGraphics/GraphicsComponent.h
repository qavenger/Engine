#pragma once
#include "../Shared/pch.h"
#include "GraphicsDefine.h"
struct SubMesh
{
	SubMesh(){}
	SubMesh(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation) : 
		IndexCount(indexCount), 
		StartIndexLocation(startIndexLocation), 
		BaseVertexLocation(baseVertexLocation){}
	SubMesh(const SubMesh& other) : 
		IndexCount(other.IndexCount), 
		StartIndexLocation(other.StartIndexLocation), 
		BaseVertexLocation(other.BaseVertexLocation), 
		Bound(other.Bound) {}

	UINT	IndexCount = 0;
	UINT	StartIndexLocation = 0;
	UINT	BaseVertexLocation = 0;

	DirectX::BoundingBox	Bound;
};

class Mesh
{
public:
	std::wstring					Name;
	ComPtr<ID3DBlob>				VertexBufferCPU;
	ComPtr<ID3DBlob>				IndexBufferCPU;

	ComPtr<ID3D12Resource>			VertexBuffer;
	ComPtr<ID3D12Resource>			IndexBuffer;

	ComPtr<ID3D12Resource>			VertexUploader;
	ComPtr<ID3D12Resource>			IndexUploader;

	DXGI_FORMAT						IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT							IndexCount;
	std::unordered_map<std::wstring, SubMesh> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW		VertexBufferView[3];
	D3D12_INDEX_BUFFER_VIEW			IndexBufferView;

public:
	void DisposeUploader() {
		VertexUploader.Reset();
		IndexUploader.Reset();
	}
};

struct Texture
{
	std::wstring		Name;
	std::wstring		FileName;
	ComPtr<ID3D12Resource>	Resource = nullptr;
	ComPtr<ID3D12Resource>	Uploader = nullptr;
};

enum RenderQueueType
{
	SHADOW,
	OPAQUE,
	TRANSPARENT,
	POSTPROCESS,
	NUM_RENDER_QUEUE_TYPE
};

class Material
{
public:
	RenderQueueType		Type;
	std::wstring		Name;
	int					ConstantHeapIndex;
	std::vector<int>	TextureHeapIndex;
};
namespace EntityComponent
{ 
	class StaticMesh;
}
struct RenderObject
{
	float4x4*		transform;
	UINT			cbIdx;
	EntityComponent::StaticMesh*	mesh;
};

class RenderQueue
{
public:
	void Initialize();
	void ClearQueue();
	void Add(RenderQueueType type, RenderObject obj);
	std::vector<std::vector<RenderObject>> queue;
};

