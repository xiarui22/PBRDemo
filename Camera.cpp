#include "Camera.h"
#include <Windows.h>
#include <iostream>

using namespace std;

Camera::Camera()
{
}

Camera::Camera(XMFLOAT3 pos, XMFLOAT3 dirz)
{
	camPos = pos;
	camDir = dirz;
	rotationX = 0;
	rotationY = 0;
}

XMFLOAT3 Camera::GetCamPos() {
	return camPos;
}

float Camera::GetRotationx() {
	return rotationX;
}

float Camera::GetRotationy() {
	return rotationY;
}

void Camera::SetRotation(float x, float y) {
	rotationX+= x;
	rotationY+=y;

	rotationX = max(min(rotationX, XM_PIDIV2), -XM_PIDIV2);
}

XMFLOAT4X4 Camera::GetView() {
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjection() {

	return projectionMatrix;
}

void Camera::UpdateProjection(unsigned int width, unsigned int height) {

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
	
}

void Camera::Update(float totaltime) {
	
	XMVECTOR camPosv = XMLoadFloat3(&camPos);

	XMMATRIX rotation = XMMatrixRotationX(rotationX);
	rotation = XMMatrixMultiply(rotation, XMMatrixRotationY(rotationY));
	
	XMFLOAT3 zz = XMFLOAT3(0, 0, 1);
	XMVECTOR dirzz = XMLoadFloat3(&zz);

	XMVECTOR camDirv = XMVector3Transform(dirzz,rotation);

	camDir = XMFLOAT3(XMVectorGetX(camDirv), XMVectorGetY(camDirv), XMVectorGetZ(camDirv));
	XMVECTOR up = XMVectorSet(0, 1, 0,0);


	XMMATRIX V = XMMatrixLookToLH(
		camPosv,     // The position of the "camera"
		camDirv,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));


	XMVECTOR crossp= XMVector3Cross(camDirv, up);
	XMFLOAT3 lft = XMFLOAT3(XMVectorGetX(crossp), XMVectorGetY(crossp), XMVectorGetZ(crossp));
	//cout << "ss";


	// the calculation!!!!!    where to put???? camera update or game update(delta time??) 
	if (GetAsyncKeyState('W') & 0x8000) {
		
		camPos.x = camPos.x + camDir.x*totaltime; 
		camPos.y = camPos.y + camDir.y*totaltime;
		camPos.z = camPos.z + camDir.z*totaltime;

		//camPosv = XMVectorAdd(camPosv, camDirv);
		
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		
		camPos.x = camPos.x - camDir.x*totaltime;
		camPos.y = camPos.y - camDir.y*totaltime;
		camPos.z = camPos.z - camDir.z*totaltime;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		
		camPos.x = camPos.x + lft.x*totaltime;
		camPos.y = camPos.y + lft.y*totaltime;
		camPos.z = camPos.z + lft.z*totaltime;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		camPos.x = camPos.x - lft.x*totaltime;
		camPos.y = camPos.y - lft.y*totaltime;
		camPos.z = camPos.z - lft.z*totaltime;
	}
	if (GetAsyncKeyState('X') & 0x8000) {
		camPos.y -= totaltime;
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		camPos.y += totaltime;
	}
	


}



Camera::~Camera()
{
}
