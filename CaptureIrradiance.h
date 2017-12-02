#pragma once
#include <d3d11.h>
#include "SimpleShader.h"
#include "Entity.h"
class CaptureIrradiance
{
	SimpleVertexShader * irradianceVS;
	SimplePixelShader * irradiancePS;

	ID3D11Texture2D * irradianceMap;
	ID3D11RenderTargetView * capturedRTV[6];
	ID3D11ShaderResourceView * capturedSRV;

	XMFLOAT4X4 captureViews[6];
	XMFLOAT4X4 captureProjection;

	int width;
	int height;

	ID3D11Resource * captureRTVResource;
	ID3D11DepthStencilView*  captureDSV;

	//ID3D11RenderTargetView * test;
public:
	CaptureIrradiance();
	~CaptureIrradiance();
	bool Init(ID3D11Device *, int, int);
	void SetRenderTarget(ID3D11RenderTargetView *,ID3D11DeviceContext *);
	void ClearRenderTarget(ID3D11RenderTargetView *,ID3D11DeviceContext*);

	ID3D11ShaderResourceView * GetShaderResourceView();
	ID3D11Texture2D * GetIrradianceMap();

	void RenderEnvironmentMap(ID3D11DeviceContext *, Entity *);
	void CreateCaptureMatrices();
};

