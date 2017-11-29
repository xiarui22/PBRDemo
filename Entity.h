#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

using namespace DirectX;
class Entity
{
	Mesh *mesh;
	Material *material;

	XMFLOAT4X4 worldMatrix;
	XMFLOAT3 translation;
	XMFLOAT3 rotate;
	XMFLOAT3 scale;
public:
	Entity();
	Entity(Mesh *, Material *);
	~Entity();

	void setTranslation(float, float, float);
	void setScale(float, float, float);
	void setRotate(float, float, float);
	XMFLOAT3 getTranslation();
	XMFLOAT3 getScale();
	XMFLOAT3 getRotate();

	void setWorld(XMFLOAT3, XMFLOAT3, XMFLOAT3);
	XMFLOAT4X4 getWorld();

	Mesh * getMesh();
	Material * getMaterial();

};

