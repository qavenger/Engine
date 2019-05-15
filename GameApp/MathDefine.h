#pragma once
#include <DirectXMath.h>
using DirectX::XMVECTOR;
#define FLT_EQUALS_ZERO(a)		(fabsf(a)<FLT_EPSILON)
#define G_FORWARD				XMVectorSet(0,0,1,0)
#define G_UP					XMVectorSet(0,1,0,0)
#define G_RIGHT					XMVectorSet(1,0,0,0)

#define G_CLAMP(low,high,a)		fminf(fmaxf(a, low),high)
#define G_CLAMP01(a)			G_CLAMP(0,1,a)
#define G_DEG2RAD				0.0174532925f
#define G_RAD2DEG				57.295779513f
#define G_GRAVITY				XMVectorSet(0,-9.80665f,0,0)
#define G_EPISILON				XMVectorSet(1e-2f, 1e-2f, 1e-2f, 1e-2f)