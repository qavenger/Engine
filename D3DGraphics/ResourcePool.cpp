#include "../Shared/pch.h"
#include "ResourcePool.h"
#include "DDSTextureLoader.h"
#include "../FileUtility/FileUtility.h"

std::vector<Texture> TexturePool::m_texture;
std::unordered_map<std::wstring, Texture*> TexturePool::m_textureLookup;
std::unordered_map<std::wstring, Mesh*> MeshPool::meshes;

void TexturePool::Initialize()
{
	LoadTextures(L"../GameApp/Asset/Texture");
}

void TexturePool::Shutdown()
{
	m_texture.clear();
	m_textureLookup.clear();
}

Texture * TexturePool::FindTextureByName(const wchar_t * name)
{
	auto itr = m_textureLookup.find(name);
	if (itr == m_textureLookup.end())return nullptr;
	return itr->second;
}

void TexturePool::Disposal()
{
	for (int i = 0; i < (int)m_texture.size(); ++i)
	{
		m_texture[i].Uploader.Reset();
	}
}

void TexturePool::LoadTextures(const wchar_t * path)
{
	std::wstring searchPath(path);
	FileUtility fileFunc;
	fileFunc.TrimDelimiter(searchPath);
	if (searchPath[searchPath.length() - 1] != L'/')
		searchPath += L"/*.*";

	fileFunc.SearchFileInFolder(searchPath.data(), L".dds", [&](const wchar_t* filename) {
		Texture texture;
		texture.FileName = filename;
		fileFunc.TrimDelimiter(texture.FileName);

		CreateDDSTextureFromFile12(D3DDevice::pDevice, D3DDevice::pCmdList, filename, texture.Resource, texture.Uploader);
		auto start = texture.FileName.find_last_of(L'/') + 1;
		auto end = texture.FileName.find_last_of(L'.');
		auto count = end - start;
		texture.Name = texture.FileName.substr(start, count);
		if (texture.Resource.Get())
		{
			m_texture.push_back(texture);
			m_textureLookup[texture.Name] = &m_texture[m_texture.size() - 1];
		}
	});
}


void MeshPool::Shutdown()
{
	for (auto itr = meshes.begin(); itr != meshes.end(); ++itr)
	{
		delete itr->second;
	}
	meshes.clear();
}

void MeshPool::Disposal()
{
	for (auto itr = meshes.begin(); itr != meshes.end(); ++itr)
	{
		itr->second->DisposeUploader();
	}
}
