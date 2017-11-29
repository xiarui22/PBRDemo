#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct DirectionalLight {
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;
};

struct PointLight {
	XMFLOAT4 pointLightColor;
	XMFLOAT3 pointLightPosition;
	float i;
};