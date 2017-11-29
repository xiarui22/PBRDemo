#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>

enum MaterialType { kMaterialNormal, kMaterialCubemap, kMaterialPBR };

class Material
{
	SimpleVertexShader * vertexShader;
	SimplePixelShader * pixelShader;

	ID3D11ShaderResourceView * srv;
	ID3D11SamplerState *samplerState;
	MaterialType type;
	ID3D11RasterizerState* rsSky;
	ID3D11DepthStencilState* dsSky;

	//PBR shader resource view
	ID3D11ShaderResourceView * albedoSrv;
	ID3D11ShaderResourceView * metallicSrv;
	ID3D11ShaderResourceView * roughnessSrv;
	ID3D11ShaderResourceView * aoSrv;
	ID3D11ShaderResourceView * normalSrv;

public:
	Material(ID3D11Device * device, ID3D11DeviceContext * context, MaterialType type, const wchar_t * path, 
		const wchar_t *albedopath, const wchar_t *metallicpath, const wchar_t *roughnesspath, const wchar_t *aopath, const wchar_t * normalpath);
	Material(SimpleVertexShader *, SimplePixelShader *, ID3D11ShaderResourceView *, ID3D11SamplerState *);
	~Material();

	SimpleVertexShader *GetvertexShader();
	SimplePixelShader *GetpixelShader();
	ID3D11ShaderResourceView * GetShaderResourceView();
	ID3D11SamplerState * GetSamplerState();
	ID3D11RasterizerState* GetRastState();
	ID3D11DepthStencilState* GetDepthStencilState();


	void SetSamplerState(ID3D11SamplerState *);
	void SetShaderResource(ID3D11ShaderResourceView *);
	void SetTexture(ID3D11Device * , ID3D11DeviceContext * , const wchar_t * , const wchar_t * );
	void SetupSkybox(ID3D11Device *, ID3D11DeviceContext *, const wchar_t *);
	void SetPBRTexture(ID3D11Device *, ID3D11DeviceContext *, const wchar_t *, const wchar_t *, const wchar_t *, const wchar_t *, const wchar_t *);

	void LoadVertexShaders(ID3D11Device * , ID3D11DeviceContext * , const wchar_t *);
	void LoadPixelShaders(ID3D11Device *, ID3D11DeviceContext *, const wchar_t *);

	void SetVertexShaderMatrix(DirectX::XMFLOAT4X4 worldm, DirectX::XMFLOAT4X4 viewm, DirectX::XMFLOAT4X4 projectionm);
	void SetPixelShaderSrv();
	void SetSkyPixelShaderSrv();
	void SetPBRPixelShaderSrv();

};

