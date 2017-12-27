#pragma once
#include <d3d11.h>
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "SpriteBatch.h"
#include "Entity.h"

class CaptureTexture2d
{
	SimpleVertexShader * texture2dVS;
	SimplePixelShader * texture2dPS;

	ID3D11Texture2D * texture2d;

	ID3D11RenderTargetView * capturedRTV;
	ID3D11ShaderResourceView * capturedSRV;

	DirectX::XMFLOAT4X4 captureView;
	DirectX::XMFLOAT4X4 captureProjection;

	int width;
	int height;

	ID3D11Resource * captureRTVResource;
	ID3D11DepthStencilView*  captureDSV;

	

public:
	CaptureTexture2d();
	~CaptureTexture2d();

	bool Init(ID3D11Device *, ID3D11DeviceContext *, int, int);
	void SetRenderTarget(ID3D11DeviceContext *);
	void ClearRenderTarget(ID3D11DeviceContext*);

	ID3D11ShaderResourceView * GetShaderResourceView();
	ID3D11Texture2D * GetTexture2d();

	void RenderBrdfLUT(ID3D11DeviceContext *, Entity *);
	void CreateCaptureMatrices();

	bool SaveBrdfLUT(ID3D11Device *, ID3D11DeviceContext *);
	bool BrdfLUTExists(ID3D11Device *, ID3D11DeviceContext *);

};

