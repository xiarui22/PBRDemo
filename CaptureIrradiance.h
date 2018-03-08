#pragma once
#include <d3d11.h>
#include "SimpleShader.h"
#include "Entity.h"
class CaptureIrradiance
{
	SimpleVertexShader * irradianceVS;
	SimplePixelShader * irradiancePS;

	ID3D11Texture2D * irradianceMap;

	ID3D11RenderTargetView * capturedRTV[5][6];
	ID3D11ShaderResourceView * capturedSRV;

	XMFLOAT4X4 captureViews[6];
	XMFLOAT4X4 captureProjection;

	int width;
	int height;

	ID3D11Resource * captureRTVResource;
	ID3D11DepthStencilView*  captureDSV[5]; // For each mip level

public:
	CaptureIrradiance();
	~CaptureIrradiance();
	bool Init(ID3D11Device *, ID3D11DeviceContext *, int, int);
	void SetRenderTarget(ID3D11RenderTargetView *,ID3D11DeviceContext *, ID3D11DepthStencilView *);
	void ClearRenderTarget(ID3D11RenderTargetView *,ID3D11DeviceContext*,  ID3D11DepthStencilView *);

	ID3D11ShaderResourceView * GetShaderResourceView();
	ID3D11Texture2D * GetIrradianceMap();

	void RenderEnvironmentDiffuseMap(ID3D11DeviceContext *, Entity *);
	void RenderPrefilteredMap(ID3D11DeviceContext *, Entity *);
	void CreateCaptureMatrices();

	bool SaveEnvironmentDiffuseMap(ID3D11Device *, ID3D11DeviceContext *);
	bool EnvironmentDiffuseMapExists(ID3D11Device *);

	bool SavePrefilteredMap(ID3D11Device *, ID3D11DeviceContext *);
	bool PrefilteredMapExists(ID3D11Device *);

};

