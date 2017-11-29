#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Entity.h"

class Camera
{
	
	XMFLOAT3 camPos;
	XMFLOAT3 camDir;
	float rotationX;
	float rotationY;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

public:
	Camera();
	Camera(XMFLOAT3, XMFLOAT3);
	~Camera();

	XMFLOAT3 GetCamPos();

	float GetRotationx();
	float GetRotationy();

	XMFLOAT4X4 GetView();  //look to
	XMFLOAT4X4 GetProjection();
	void UpdateProjection(unsigned int, unsigned int);

	//void Update(XMFLOAT3, XMFLOAT3, float, float);
	void Update(float);

	void SetRotation(float, float);
};

