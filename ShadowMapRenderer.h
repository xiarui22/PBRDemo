#pragma once
#pragma once
#include <d3d11.h>
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <vector>
#include "Scene.h"


class ShadowMapRenderer
{
	SimpleVertexShader * shadowMapVS;
	SimplePixelShader * shadowMapPS;

	ID3D11Texture2D * depthMap;

	ID3D11ShaderResourceView * capturedSRV;

    // So far only one light source, fixed me:
	DirectX::XMFLOAT3 lightPosition;            


	int width;
	int height;

	ID3D11Resource * captureRTVResource;
	ID3D11DepthStencilView*  captureDSV;

	ID3D11SamplerState* shadowSampler;
	ID3D11RasterizerState *shadowRasterizer;

public:
	ShadowMapRenderer();
	~ShadowMapRenderer();

	bool Init(ID3D11Device *, ID3D11DeviceContext *, int, int);

	void SetRenderTarget(ID3D11DeviceContext *);
	void ClearRenderTarget(ID3D11DeviceContext*);

	ID3D11ShaderResourceView * GetShaderResourceView();
	ID3D11Texture2D * GetTexture2d();
	ID3D11SamplerState* GetShadowSampler();

	void RenderDepthMap(ID3D11DeviceContext *, Scene *);
	void CreateMatrices(DirectX::XMFLOAT3);

	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
};

