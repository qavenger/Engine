#pragma once
#include "GraphicsComponent.h"
class TexturePool
{
public:
	static void Initialize();
	static void Shutdown();
	~TexturePool() { Shutdown(); }
	static Texture* GetTexture(UINT index) { if (index >= m_texture.size()) return nullptr; return &m_texture[index]; }
	static Texture* FindTextureByName(const wchar_t* name);
	static void Disposal();
	static UINT GetNumTextures() { return (UINT)m_texture.size(); }
private:
	static void LoadTextures(const wchar_t* path);
private:
	static std::vector<Texture> m_texture;
	static std::unordered_map<std::wstring, Texture*> m_textureLookup;
};

class MeshPool
{
public:
	static std::unordered_map<std::wstring, Mesh*> meshes;
	~MeshPool() { if (meshes.size() > 0) Shutdown(); }
	static void Shutdown();
	static void Disposal();
};

