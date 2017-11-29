#pragma once
#include <d3d11.h>
class CaptureIrradiance
{
	ID3D11Texture2D * irradianceMap;
	ID3D11RenderTargetView * capturedRTV;
	ID3D11ShaderResourceView * capturedSRV;

public:
	CaptureIrradiance();
	~CaptureIrradiance();
	bool Init(ID3D11Device *, int, int);
	void RenderToTexuture();
};

