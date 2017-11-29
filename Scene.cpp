#include "Scene.h"

Scene::Scene()
{
}


Scene::~Scene()
{
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			delete entities[i][j];

	delete mesh;
	delete PBRmaterial;
	delete skyBoxMaterial;
	delete skyBoxMesh;
	delete skyBox;
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Scene::CreateBasicGeometry(ID3D11Device * device)
{
	// Create the VERTEX BUFFER description -----------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	mesh = new Mesh("Assets/Models/sphere.obj", device);
	skyBoxMesh = new Mesh("Assets/Models/cube.obj", device);
}

void Scene::CreateMaterial(ID3D11Device * device, ID3D11DeviceContext * context)
{
	skyBoxMaterial = new Material(device, context, kMaterialCubemap, L"Assets/Textures/hw_crater.dds", nullptr, nullptr, nullptr, nullptr, nullptr);
	skyBoxMaterial->LoadVertexShaders(device, context, L"SkyVS");
	skyBoxMaterial->LoadPixelShaders(device, context, L"SkyPS");
	//skyBoxMaterial->LoadVertexShaders(device, context, L"ConvolutionVertexShader");
	//skyBoxMaterial->LoadPixelShaders(device, context, L"ConvolutionPixelShader");

	PBRmaterial = new Material(device, context, kMaterialPBR, nullptr, L"Assets/Textures/greasy-pan-2-albedo.png", L"Assets/Textures/greasy-pan-2-metal.png",
		L"Assets/Textures/parameter0.png", L"Assets/Textures/parameter1.png", L"Assets/Textures/greasy-pan-2-normal.png");
	PBRmaterial->LoadVertexShaders(device, context, L"PBRVertexShader");
	PBRmaterial->LoadPixelShaders(device, context, L"PBRPixelShader");
}

void Scene::CreateLights()
{
	pointLight0.pointLightColor = XMFLOAT4(1, 1, 1, 1);
	pointLight0.pointLightPosition = XMFLOAT3(5, 0, -3);

	pointLight1.pointLightColor = XMFLOAT4(1, 1, 1, 1);
	pointLight1.pointLightPosition = XMFLOAT3(0, 5, -3);

	pointLight2.pointLightColor = XMFLOAT4(1, 1, 1, 1);
	pointLight2.pointLightPosition = XMFLOAT3(5, 5, -3);

	pointLight3.pointLightColor = XMFLOAT4(1, 1, 1, 1);
	pointLight3.pointLightPosition = XMFLOAT3(0, 0, -3);

	albedo = XMFLOAT3(1,0,0);
	for (int i = 0; i < 5; i++) {
		metallic[i] = 0.1+0.2*i;
		roughness[i] = 0.1+0.2*i;
	}
	ao = 0.5f;
}

void Scene::CreateEntities()
{
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			entities[i][j] = new Entity(mesh, PBRmaterial);

	skyBox = new Entity(skyBoxMesh, skyBoxMaterial);
}

void Scene::init(ID3D11Device * device, ID3D11DeviceContext * context)
{
	CreateBasicGeometry(device);
	CreateMaterial(device, context);
	CreateEntities();
	CreateLights();
}
