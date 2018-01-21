#pragma once

#include "Material.h"
#include "Light.h"
#include "Mesh.h"
#include "Entity.h"

class Scene
{
	Mesh *mesh;
	Mesh * skyBoxMesh;
	Mesh * plane2dMesh;
	Material *PBRmaterial;
	Material *skyBoxMaterial;
	Material *enviDiffuseMaterial;
	Material *prefilteredMaterial;
	Material *brdfLUTMaterial;

	
public:
	Scene();
	~Scene();
    Entity * spheres[5][5];
	Entity * quads[2];
	PointLight pointLight0;
	PointLight pointLight1;
	PointLight pointLight2;
	PointLight pointLight3;
	Entity * skyBox;

	// use for shadow map
	std::vector<Entity *> entitiesOpaque;
	Material *shadowMap;


	// Actually another scene, there's a cube to draw the skybox on and to be captured to generate irradiance
	Entity * cubeForCaptureEnviDiffuse;
	Entity * cubeForCapturePrefiltered;
	Entity * brdfLUT;

	// use for PBR
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

