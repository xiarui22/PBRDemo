#pragma once

#include "Material.h"
#include "Light.h"
#include "Mesh.h"
#include "Entity.h"
class Scene
{
	Mesh *mesh;
	Mesh * skyBoxMesh;
	//Material *material;
	Material *PBRmaterial;
	Material *skyBoxMaterial;
	
public:
	Scene();
	~Scene();
    Entity * entities[5][5];
	PointLight pointLight0;
	PointLight pointLight1;
	PointLight pointLight2;
	PointLight pointLight3;
	Entity * skyBox;

	//use for PBR
	XMFLOAT3 albedo;
	float metallic[5];
	float roughness[5];
	float ao;

	// Initialization helper methods - feel free to customize, combine, etc.
	void CreateBasicGeometry(ID3D11Device *);
	void CreateMaterial(ID3D11Device *, ID3D11DeviceContext *);
	void CreateLights();
	void CreateEntities();

	void init(ID3D11Device *, ID3D11DeviceContext *);
};

