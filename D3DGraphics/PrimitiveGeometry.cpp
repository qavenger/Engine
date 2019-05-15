#include "../Shared/pch.h"
#include "PrimitiveGeometry.h"

PrimitiveGeometry::MeshData PrimitiveGeometry::CreatePlane(float width, float depth, UINT widthSigmentCount, UINT depthSigmentCount, bool isVertical)
{
	MeshData mesh;

	float dW = width / widthSigmentCount;
	float dD = depth / depthSigmentCount;

	float startX = -width * 0.5f;
	float startZ = -depth * 0.5f;
	float invW = 1 / width;
	float invD = 1 / depth;
	Vertex v;
	v.addition.color = { 1,1,1,1 };
	v.addition.normal = { 0,1,0 };

	float x, y = startZ;
	for (UINT j = 0; j <= depthSigmentCount; ++j)
	{
		x = startX;
		for (UINT i = 0; i <= widthSigmentCount; ++i)
		{
			v.base.pos = { x, 0, y };
			v.base.uv = { x * invW + 0.5f,1 - y * invD - 0.5f };
			x += dW;
			mesh.Vertex.push_back(v);
		}
		y += dD;
	}

	UINT YVertCount = depthSigmentCount + 1;
	UINT v0, v1, v2, v3;
	for (UINT j = 0; j < depthSigmentCount; ++j)
	{
		for (UINT i = 0; i < widthSigmentCount; ++i)
		{
			v0 = i + j * YVertCount;
			v1 = (i + 1) + j * YVertCount;
			v2 = i + (j + 1) * YVertCount;
			v3 = (i + 1) + (j + 1) * YVertCount;
			mesh.Index32.push_back(v0);
			mesh.Index32.push_back(v2);
			mesh.Index32.push_back(v3);

			mesh.Index32.push_back(v3);
			mesh.Index32.push_back(v1);
			mesh.Index32.push_back(v0);
		}
	}
	ComputeTangent(mesh);
	return mesh;
}

PrimitiveGeometry::MeshData PrimitiveGeometry::CreateBox(float width, float height, float depth, int numSubdivisions)
{
	MeshData mesh;

	Vertex v[24];

	float hW = width * 0.5f;
	float hH = height * 0.5f;
	float hD = depth * 0.5f;

	/*
	  0 o------o 1
		|     /|
		|   /  |
		| /    |
	  2	o------o 3
	*/

	//front face
	v[ 0].base = { { -hW, +hH, -hD },{ 0, 0 } };
	v[ 1].base = { { +hW, +hH, -hD },{ 1, 0 } };
	v[ 2].base = { { -hW, -hH, -hD },{ 0, 1 } };
	v[ 3].base = { { +hW, -hH, -hD },{ 1, 1 } };

	v[ 0].addition = { { 1, 1, 1, 1 },{ +0, +0, -1 }, {  } };
	v[ 1].addition = { { 1, 1, 1, 1 },{ +0, +0, -1 }, {  } };
	v[ 2].addition = { { 1, 1, 1, 1 },{ +0, +0, -1 }, {  } };
	v[ 3].addition = { { 1, 1, 1, 1 },{ +0, +0, -1 }, {  } };

	//right face
	v[ 4].base = { { +hW, +hH, -hD },{ 0, 0 } };
	v[ 5].base = { { +hW, +hH, +hD },{ 1, 0 } };
	v[ 6].base = { { +hW, -hH, -hD },{ 0, 1 } };
	v[ 7].base = { { +hW, -hH, +hD },{ 1, 1 } };

	v[ 4].addition = { { 1, 1, 1, 1 },{ +1, +0, +0 },{} };
	v[ 5].addition = { { 1, 1, 1, 1 },{ +1, +0, +0 },{} };
	v[ 6].addition = { { 1, 1, 1, 1 },{ +1, +0, +0 },{} };
	v[ 7].addition = { { 1, 1, 1, 1 },{ +1, +0, +0 },{} };

	//back face
	v[ 8].base = { { +hW, +hH, +hD },{ 0, 0 } };
	v[ 9].base = { { -hW, +hH, +hD },{ 1, 0 } };
	v[10].base = { { +hW, -hH, +hD },{ 0, 1 } };
	v[11].base = { { -hW, -hH, +hD },{ 1, 1 } };

	v[ 8].addition = { { 1, 1, 1, 1 },{ +0, +0, +1 },{} };
	v[ 9].addition = { { 1, 1, 1, 1 },{ +0, +0, +1 },{} };
	v[10].addition = { { 1, 1, 1, 1 },{ +0, +0, +1 },{} };
	v[11].addition = { { 1, 1, 1, 1 },{ +0, +0, +1 },{} };

	//left face
	v[12].base = { { -hW, +hH, +hD },{ 0, 0 } };
	v[13].base = { { -hW, +hH, -hD },{ 1, 0 } };
	v[14].base = { { -hW, -hH, +hD },{ 0, 1 } };
	v[15].base = { { -hW, -hH, -hD },{ 1, 1 } };

	v[12].addition = { { 1, 1, 1, 1 },{ -1, +0, +0 },{} };
	v[13].addition = { { 1, 1, 1, 1 },{ -1, +0, +0 },{} };
	v[14].addition = { { 1, 1, 1, 1 },{ -1, +0, +0 },{} };
	v[15].addition = { { 1, 1, 1, 1 },{ -1, +0, +0 },{} };

	//top face
	v[16].base = { { -hW, +hH, +hD },{ 0, 0 } };
	v[17].base = { { +hW, +hH, +hD },{ 1, 0 } };
	v[18].base = { { -hW, +hH, -hD },{ 0, 1 } };
	v[19].base = { { +hW, +hH, -hD },{ 1, 1 } };

	v[16].addition = { { 1, 1, 1, 1 },{ +0, +1, +0 },{} };
	v[17].addition = { { 1, 1, 1, 1 },{ +0, +1, +0 },{} };
	v[18].addition = { { 1, 1, 1, 1 },{ +0, +1, +0 },{} };
	v[19].addition = { { 1, 1, 1, 1 },{ +0, +1, +0 },{} };

	//bottom face
	v[20].base = { { -hW, -hH, -hD },{ 0, 0 } };
	v[21].base = { { +hW, -hH, -hD },{ 1, 0 } };
	v[22].base = { { -hW, -hH, +hD },{ 0, 1 } };
	v[23].base = { { +hW, -hH, +hD },{ 1, 1 } };

	v[20].addition = { { 1, 1, 1, 1 },{ +0, -1, +0 },{} };
	v[21].addition = { { 1, 1, 1, 1 },{ +0, -1, +0 },{} };
	v[22].addition = { { 1, 1, 1, 1 },{ +0, -1, +0 },{} };
	v[23].addition = { { 1, 1, 1, 1 },{ +0, -1, +0 },{} };

	uint32_t index[36];
	int start = 0;
	int i;
	for (i = 0; i < 36; i += 6)
	{
		index[i + 0] = start + 0;
		index[i + 1] = start + 1;
		index[i + 2] = start + 2;
		index[i + 3] = start + 1;
		index[i + 4] = start + 3;
		index[i + 5] = start + 2;
		start += 4;
	}

	mesh.Vertex.assign(v, v + 24);
	mesh.Index32.assign(index, index + 36);

	ComputeTangent(mesh);

	numSubdivisions = std::min<int>(numSubdivisions, 6);

	for (i = 0; i < numSubdivisions; ++i)
	{
		Subdivide(mesh);
	}

	return mesh;
}

PrimitiveGeometry::MeshData PrimitiveGeometry::CreateCylinder(float radiusTop, float radiusBtn, float height, UINT sliceCount, UINT stackCount)
{
	MeshData mesh;
	float sliceCountInv = 1.0f / sliceCount;
	float stackCountInv = 1.0f / stackCount;

	float dh = height * stackCountInv;
	float diffR = radiusTop - radiusBtn;
	float dr = diffR * stackCountInv;
	UINT ringCount = stackCount + 1;
	UINT ringVertCount = sliceCount + 1;
	float y = -0.5f;
	float r = radiusBtn;
	float dt = 2 * XM_PI * sliceCountInv;
	Vertex v;
	UINT i, j;
	for (i = 0; i < ringCount; ++i)
	{
		for (j = 0; j < ringVertCount; ++j)
		{
			float angle = j * dt;
			float x = cosf(angle);
			float z = sinf(angle);
			v.base.pos = float3(x * r, y, z * r);
			v.base.uv = float2(j * sliceCountInv, 1.0f - i * stackCountInv);
			v.addition.tangent = float3(-z, 0, x);
			XMVECTOR T = XMLoadFloat3(&v.addition.tangent);
			XMVECTOR B = XMVectorSet(diffR * x, -height, diffR * z, 0);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&v.addition.normal, N);
			v.addition.color = float4(1, 1, 1, 1);
			mesh.Vertex.push_back(v);
		}
		y += dh;
		r += dr;
	}
	
	for (i = 0; i < stackCount; ++i)
	{
		for (j = 0; j < sliceCount; ++j)
		{
			mesh.Index32.push_back(i * ringVertCount + j);
			mesh.Index32.push_back((i + 1) * ringVertCount + j);
			mesh.Index32.push_back((i + 1) * ringVertCount + j + 1);

			mesh.Index32.push_back(i * ringVertCount + j);
			mesh.Index32.push_back((i + 1) * ringVertCount + j + 1);
			mesh.Index32.push_back(i * ringVertCount + j + 1);
		}
	}

	float topRatio = 1;
	float btnRatio = 1;
	if (radiusTop > radiusBtn)
	{
		btnRatio = radiusBtn / radiusTop;
	}
	else
	{
		topRatio = radiusTop / radiusBtn;
	}

	BuildCylinderCap(radiusTop, topRatio, true, 0.5f, sliceCount, mesh);
	BuildCylinderCap(radiusBtn, btnRatio, false, -0.5f, sliceCount, mesh);
	ComputeTangent(mesh);
	return mesh;
}

PrimitiveGeometry::MeshData PrimitiveGeometry::CreateSphere(float radius, UINT sliceCount, UINT stackCount)
{
	MeshData mesh;

	Vertex top, btn, v;
	top.base.pos = { 0, +radius, 0 };
	top.base.uv = { 0.5f, 0 };
	top.addition.normal = { 0, +1, 0 };
	top.addition.tangent = { 1, 0, 0 };
	top.addition.color = { 1,1,1,1 };
	btn.base.pos = { 0, -radius, 0 };
	btn.base.uv = { 0.5f, 1 };
	btn.addition.normal = { 0, -1, 0 };
	btn.addition.color = { 1,1,1,1 };
	btn.addition.tangent = { 1, 0, 0 };

	mesh.Vertex.push_back(top);
	float dPhi = XM_PI / stackCount;
	float dTheta = 2 * XM_PI / sliceCount;

	float phi = dPhi;
	float theta;

	v.addition.color = { 1,1,1,1 };

	for (UINT i = 1; i < stackCount; ++i)
	{
		theta = 0;
		for (UINT j = 0; j <= sliceCount; ++j)
		{	
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			v.base.pos = { sinPhi * cosTheta * radius , cosPhi * radius , sinTheta * sinPhi * radius };
			v.addition.tangent = { -radius * sinPhi * sinTheta ,0, radius * sinPhi * cosTheta };
			XMVECTOR T = XMLoadFloat3(&v.addition.tangent);
			XMVECTOR N = XMLoadFloat3(&v.base.pos);
			XMStoreFloat3(&v.addition.tangent, XMVector3Normalize(T));
			XMStoreFloat3(&v.addition.normal, XMVector3Normalize(N));
			v.base.uv = { theta * XM_1DIV2PI, phi * XM_1DIVPI};
			mesh.Vertex.push_back(v);
			theta += dTheta;
		}
		phi += dPhi;
	}
	mesh.Vertex.push_back(btn);

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		mesh.Index32.push_back(0);
		mesh.Index32.push_back(i + 1);
		mesh.Index32.push_back(i);
	}

	UINT baseIdx = 1;
	UINT ringVertCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			mesh.Index32.push_back(baseIdx + i * ringVertCount + j);
			mesh.Index32.push_back(baseIdx + i * ringVertCount + j + 1);
			mesh.Index32.push_back(baseIdx + (i + 1)*ringVertCount + j);

			mesh.Index32.push_back(baseIdx + (i + 1)*ringVertCount + j);
			mesh.Index32.push_back(baseIdx + i * ringVertCount + j + 1);
			mesh.Index32.push_back(baseIdx + (i + 1)*ringVertCount + j + 1);
		}
	}

	UINT btnIdx = (UINT)mesh.Vertex.size() - 1;
	baseIdx = btnIdx - ringVertCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		mesh.Index32.push_back(btnIdx);
		mesh.Index32.push_back(baseIdx + i);
		mesh.Index32.push_back(baseIdx + i + 1);
	}

	ComputeTangent(mesh);
	return mesh;
}

PrimitiveGeometry::MeshData PrimitiveGeometry::CreateGeoSphere(float radius, UINT numSubdivisions)
{
	MeshData mesh;

	numSubdivisions = std::min<int>(numSubdivisions, 6);
	const float x = 0.525731f;
	const float z = 0.850651f;

	float3 pos[12] =
	{
		{-x, 0, z}, { x, 0, z},
		{-x, 0,-z}, { x, 0,-z},
		{ 0, z, x}, { 0, z,-x},
		{ 0,-z, x}, { 0,-z,-x},
		{ z, x, 0}, {-z, x, 0},
		{ z,-x, 0}, {-z,-x, 0}
	};

	UINT idx[60]
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	mesh.Vertex.resize(12);
	mesh.Index32.assign(idx, idx + 60);
	UINT i;
	for (i = 0; i < 12; ++i)
	{
		mesh.Vertex[i].base.pos = pos[i];
	}
	for (i = 0; i < numSubdivisions; ++i)
	{
		Subdivide(mesh);
	}
	float theta;
	float phi;
	float invR = 1 / radius;
	for (i = 0; i < mesh.Vertex.size(); ++i)
	{
		Vertex& v = mesh.Vertex[i];
		XMVECTOR N = XMVector3Normalize( XMLoadFloat3(&v.base.pos) );
		XMVECTOR P = N * radius;

		XMStoreFloat3(&v.base.pos, P);
		XMStoreFloat3(&v.addition.normal, N);
		theta = atan2f(v.base.pos.z, v.base.pos.x);
		theta += XM_2PI * (theta < 0);
		phi = acosf(v.base.pos.y * invR);
		v.base.uv = { theta * XM_1DIV2PI, phi*XM_1DIVPI };
		v.addition.color = { 1,1,1,1 };
	}
	ComputeTangent(mesh);
	return mesh;
}

void PrimitiveGeometry::Subdivide(MeshData & data) 
{
	MeshData copy = std::move(data);
	uint32_t numTris = (uint32_t)copy.Index32.size() / 3;

	Vertex m0, m1, m2;
	data.Vertex.reserve(numTris * 6);
	//12 -- 3 indices * 4 Triangles
	data.Index32.reserve(12 * numTris);

	//       v0
	//        o
	//       / \
	//      / A \
	//  m2 o-----o m0
	//    / \ C / \
	//   / B \ / D \
	//  o-----o-----o
	// v2    m1     v1

	for (uint32_t i = 0; i < numTris; ++i)
	{
		const Vertex& v0 = copy.Vertex[copy.Index32[i * 3 + 0]];
		const Vertex& v1 = copy.Vertex[copy.Index32[i * 3 + 1]];
		const Vertex& v2 = copy.Vertex[copy.Index32[i * 3 + 2]];

		m0 = MidPoint(v0, v1);
		m1 = MidPoint(v1, v2);
		m2 = MidPoint(v2, v0);

		data.Vertex.push_back(v0); // 0
		data.Vertex.push_back(v1); // 1
		data.Vertex.push_back(v2); // 2
		data.Vertex.push_back(m0); // 3
		data.Vertex.push_back(m1); // 4
		data.Vertex.push_back(m2); // 5
		
		//A
		data.Index32.push_back(i * 6 + 0);
		data.Index32.push_back(i * 6 + 3);
		data.Index32.push_back(i * 6 + 5);
		//B
		data.Index32.push_back(i * 6 + 5);
		data.Index32.push_back(i * 6 + 4);
		data.Index32.push_back(i * 6 + 2);
		//C
		data.Index32.push_back(i * 6 + 3);
		data.Index32.push_back(i * 6 + 4);
		data.Index32.push_back(i * 6 + 5);
		//D
		data.Index32.push_back(i * 6 + 3);
		data.Index32.push_back(i * 6 + 1);
		data.Index32.push_back(i * 6 + 4);
	}
}

PrimitiveGeometry::Vertex PrimitiveGeometry::MidPoint(const Vertex & v0, const Vertex & v1)
{
	Vertex m;
	XMVECTOR p0 = XMLoadFloat3(&v0.base.pos);
	XMVECTOR uv0 = XMLoadFloat2(&v0.base.uv);
	XMVECTOR c0 = XMLoadFloat4(&v0.addition.color);
	XMVECTOR n0 = XMLoadFloat3(&v0.addition.normal);
	XMVECTOR t0 = XMLoadFloat3(&v0.addition.tangent);

	XMVECTOR p1 = XMLoadFloat3(&v1.base.pos);
	XMVECTOR uv1 = XMLoadFloat2(&v1.base.uv);
	XMVECTOR c1 = XMLoadFloat4(&v1.addition.color);
	XMVECTOR n1 = XMLoadFloat3(&v1.addition.normal);
	XMVECTOR t1 = XMLoadFloat3(&v1.addition.tangent);

	XMVECTOR pm = (p0 + p1) * 0.5f;
	XMVECTOR uvm = (uv0 + uv1) * 0.5f;
	XMVECTOR cm = (c0 + c1) * 0.5f;
	XMVECTOR nm = XMVector3Normalize((n0 + n1) * 0.5f);
	XMVECTOR tm = XMVector3Normalize((t0 + t1) * 0.5f);
	XMStoreFloat3(&m.base.pos, pm);
	XMStoreFloat2(&m.base.uv, uvm);
	XMStoreFloat4(&m.addition.color, cm);
	XMStoreFloat3(&m.addition.normal, nm);
	XMStoreFloat3(&m.addition.tangent, tm);

	return m;
}

void PrimitiveGeometry::ComputeTangent(MeshData & data)
{
	uint32_t numTris = (uint32_t)data.Index32.size() / 3;

	for (uint32_t i = 0; i < numTris; ++i)
	{
		Vertex& v0 = data.Vertex[data.Index32[i * 3 + 0]];
		Vertex& v1 = data.Vertex[data.Index32[i * 3 + 1]];
		Vertex& v2 = data.Vertex[data.Index32[i * 3 + 2]];

		XMVECTOR p0 = XMLoadFloat3(&v0.base.pos);
		XMVECTOR p1 = XMLoadFloat3(&v1.base.pos);
		XMVECTOR p2 = XMLoadFloat3(&v2.base.pos);
		XMVECTOR uv0 = XMLoadFloat2(&v0.base.uv);
		XMVECTOR uv1 = XMLoadFloat2(&v1.base.uv);
		XMVECTOR uv2 = XMLoadFloat2(&v2.base.uv);

		XMVECTOR dp1 = p1 - p0;
		XMVECTOR dp2 = p2 - p0;
		XMVECTOR duv1 = uv1 - uv0;
		XMVECTOR duv2 = uv2 - uv0;

		XMVECTOR r = XMVectorSet(1,1,1,1) / XMVector2Cross(duv1, duv2);
		XMVECTOR t = (dp1 * XMVectorSwizzle(duv2, 1, 1, 1, 1) - dp2 * XMVectorSwizzle(duv1, 1, 1, 1, 1)) * r;
		XMStoreFloat3(&v0.addition.tangent, t);
		XMStoreFloat3(&v1.addition.tangent, t);
		XMStoreFloat3(&v2.addition.tangent, t);
	}
}

void PrimitiveGeometry::BuildCylinderCap(float radius, float radiusRatio, bool top, float height, UINT sliceCount, MeshData& mesh)
{
	UINT baseIdx = (UINT)mesh.Vertex.size();
	float dt = 2 * XM_PI / sliceCount;
	Vertex v;
	v.addition.normal = float3(0, top ? 1.0f : -1.0f, 0);
	//v.addition.tangent = float3(1, 0, 0);
	v.addition.color = float4(1, 1, 1, 1);
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float angle = i * dt;
		float x = cosf(angle);
		float z = sin(angle);

		v.base.pos = { x * radius, height, z*radius };
		v.base.uv = { x * 0.5f + 0.5f * radiusRatio,- z * 0.5f + 0.5f* radiusRatio };
		mesh.Vertex.push_back(v);
	}
	v.base.pos = { 0, height, 0 };
	v.base.uv = { 0.5f, 0.5f };
	mesh.Vertex.push_back(v);

	UINT center = (UINT)mesh.Vertex.size() - 1;
	for (UINT i = 0; i < sliceCount; ++i)
	{
		mesh.Index32.push_back(center);
		if (top)
		{
			mesh.Index32.push_back(baseIdx + i + 1);
			mesh.Index32.push_back(baseIdx + i);
		}
		else
		{
			mesh.Index32.push_back(baseIdx + i);
			mesh.Index32.push_back(baseIdx + i + 1);
		}
	}
}
