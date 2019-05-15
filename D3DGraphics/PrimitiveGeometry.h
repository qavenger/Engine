#pragma once
#include "GraphicsDefine.h"
class PrimitiveGeometry
{
public:
	struct Vertex
	{
		VertexBase base;
		VertexAddition addition;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertex;
		std::vector<UINT>	Index32;
		std::vector<uint16_t>& GetIndex16()
		{
			if (m_index16.empty())
			{
				m_index16.resize(Index32.size());
				for (size_t i = 0; i < m_index16.size(); ++i)
				{
					m_index16[i] = (uint16_t)Index32[i];
				}
			}
			return m_index16;
		}
	private:
		std::vector<uint16_t>	m_index16;
	};

	MeshData CreatePlane(float width, float depth, UINT widthSigmentCount, UINT depthSigmentCount, bool isVertical = false);
	MeshData CreateBox(float width, float height, float depth, int numSubdivisions);
	MeshData CreateCylinder(float radiusTop, float radiusBtn, float height, UINT sliceCount, UINT stackCount);
	MeshData CreateSphere(float radius, UINT sliceCount, UINT stackCount);
	MeshData CreateGeoSphere(float radius, UINT numSubdivisions);
private:
	void Subdivide(MeshData& data);
	Vertex MidPoint(const Vertex& p0, const Vertex& p1);
	void ComputeTangent(MeshData& data);
	void BuildCylinderCap(float radius, float radiusRatio, bool top, float height, UINT sliceCount, MeshData& mesh);
};

