#include "Entity.h"



Entity::Entity()
{
}

Entity::Entity(Mesh *pp, Material *mm)
{
	mesh = pp;
	material = mm;
	translation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(0, 0, 0);
	rotate = XMFLOAT3(0, 0, 0);
}

void Entity::setTranslation(float x, float y, float z) {
	translation = XMFLOAT3(x, y, z);
	//XMMATRIX translation = XMMatrixTranslation(x, y, z);
	//XMStoreFloat4x4(&translationMatrix, translation);
}

void Entity::setScale(float x, float y, float z) {
	scale = XMFLOAT3(x, y, z);
	//XMMATRIX scale = XMMatrixScaling(x, y, z);
	//XMStoreFloat4x4(&scaleMatrix, scale);
}
void Entity::setRotate(float x, float y, float z) {
	rotate = XMFLOAT3(x, y, z);
	//XMMATRIX scale = XMMatrixScaling(x, y, z);
	//XMStoreFloat4x4(&scaleMatrix, scale);
}
XMFLOAT3 Entity::getTranslation() {
	return translation;
}
XMFLOAT3 Entity::getScale() {
	return scale;
}
XMFLOAT3 Entity::getRotate() {
	return rotate;
}

void Entity::setWorld(XMFLOAT3 sm, XMFLOAT3 rm, XMFLOAT3 tm) {
	XMMATRIX translation = XMMatrixTranslation(tm.x, tm.y, tm.z);
	XMMATRIX scale = XMMatrixScaling(sm.x, sm.y, sm.z);
	XMMATRIX rotation = XMMatrixRotationZ(rm.z);
	rotation = XMMatrixMultiply(rotation, XMMatrixRotationX(rm.x));
	rotation = XMMatrixMultiply(rotation, XMMatrixRotationY(rm.y));

	XMVECTOR rot = XMLoadFloat3(&rm);
	XMMATRIX rotation0 = XMMatrixRotationRollPitchYawFromVector(rot);

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(scale * rotation0 * translation));
}


XMFLOAT4X4 Entity::getWorld(  ) {
	
	return worldMatrix;
}

Mesh * Entity::getMesh()
{
	return mesh;
}

Material * Entity::getMaterial()
{
	return material;
}

Entity::~Entity()
{
}
