#pragma once
#include "DXCore.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Vertex.h"
#include "DXCore.h"
#include "SimpleShader.h"


class Mesh
{
	
	ID3D11Buffer* vertexBuffer;  //vertex buffer
	ID3D11Buffer* indexBuffer;  //index buffer
	int inte = 0; //an integer specifying how many indices are in the mesh's index buffer
public:
	Mesh();
	Mesh(Vertex vertices[], int vn, unsigned int  indices[], int in, ID3D11Device * device);
	Mesh(char *, ID3D11Device * );
	
	~Mesh();
	void createBuffer(Vertex vertices[], int vn, unsigned int  indices[], int in, ID3D11Device * device);
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

};

