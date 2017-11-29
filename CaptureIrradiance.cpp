#include "CaptureIrradiance.h"



CaptureIrradiance::CaptureIrradiance()
{
	irradianceMap = 0;
	capturedRTV = 0;
	capturedSRV = 0;
}


CaptureIrradiance::~CaptureIrradiance()
{
}

bool CaptureIrradiance::Init(ID3D11Device * device , int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	//render target description
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6; //cubemap
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
    
	//create the render target texture
	result = device->CreateTexture2D(&textureDesc, NULL, &irradianceMap);
	if (FAILED(result))
	{
		return false;
	}

	//render target view description
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(irradianceMap, &rtvDesc, &capturedRTV);
	if (FAILED(result))
	{
		return false;
	}

	//shader resource view description
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(irradianceMap, &srvDesc, &capturedSRV);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

