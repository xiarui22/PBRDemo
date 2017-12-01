#include "CaptureIrradiance.h"

CaptureIrradiance::CaptureIrradiance()
{
	irradianceMap = 0;
	for (int i = 0; i < 6; i++) {
		capturedRTV[i] = 0;
	}
	capturedSRV = 0;
	captureRTVResource = 0;
}


CaptureIrradiance::~CaptureIrradiance()
{
	for (int i = 0; i < 6; i++) {
		if (capturedRTV[i]) capturedRTV[i]->Release();
	}
	if (capturedSRV) capturedSRV->Release();
	if (irradianceMap) irradianceMap->Release();
}

bool CaptureIrradiance::Init(ID3D11Device * device , int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	//texture description
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 6; //cubemap
	//textureDesc.ArraySize = 1; 
	//textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	//textureDesc.MiscFlags = 0;

	
	//create the render target texture
	result = device->CreateTexture2D(&textureDesc, 0, &irradianceMap);
	if (FAILED(result))
	{
		return false;
	}

	//render target view description
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2D.MipSlice = 0;
	for (int i = 0; i < 6; i++) {
		rtvDesc.Texture2DArray.FirstArraySlice = i;
        result = device->CreateRenderTargetView(irradianceMap, &rtvDesc, &capturedRTV[i]);
	    if (FAILED(result))
	    {
		return false;
	    }
	}


	//shader resource view description
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;
	//srvDesc.Texture2D.MipLevels = -1;
	//srvDesc.Texture2D.MostDetailedMip = 0;

	result = device->CreateShaderResourceView(irradianceMap, &srvDesc, &capturedSRV);
	if (FAILED(result))
	{
		return false;
	}

	width = textureWidth;
	height = textureHeight;



	return true;
}

void CaptureIrradiance::SetRenderTarget(ID3D11RenderTargetView * capturedRTV, ID3D11DeviceContext * context, ID3D11DepthStencilView * depthStencilView)
{
	context->OMSetRenderTargets(1, &capturedRTV, depthStencilView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void CaptureIrradiance::ClearRenderTarget(ID3D11RenderTargetView * capturedRTV, ID3D11DeviceContext * context, ID3D11DepthStencilView * depthStencilView)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	//const float color[4] = { 1, 1, 0, 0.0f };
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(capturedRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
}

ID3D11ShaderResourceView * CaptureIrradiance::GetShaderResourceView()
{
	return capturedSRV;
}

ID3D11Texture2D * CaptureIrradiance::GetIrradianceMap()
{
	return irradianceMap;
}

void CaptureIrradiance::RenderEnvironmentMap(ID3D11DeviceContext * context, ID3D11DepthStencilView * depthStencilView, Entity * cubeForCapture)
{
	
	
	
	Entity* ge = cubeForCapture;
	ID3D11Buffer* vb = ge->getMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = ge->getMesh()->GetIndexBuffer();
	CreateCaptureMatrices();	
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	for (int i = 0; i < 6; i++) {
		SetRenderTarget(capturedRTV[i], context, depthStencilView);
		ClearRenderTarget(capturedRTV[i], context, depthStencilView);

		irradianceVS = ge->getMaterial()->GetvertexShader();
		irradiancePS = ge->getMaterial()->GetpixelShader();
	
		irradianceVS->SetMatrix4x4("view", captureViews[i]);
		irradianceVS->SetMatrix4x4("projection", captureProjection);
		irradianceVS->CopyAllBufferData();
		irradianceVS->SetShader();

		irradiancePS->SetShaderResourceView("environmentMap", ge->getMaterial()->GetShaderResourceView());
		irradiancePS->SetSamplerState("basicSampler", ge->getMaterial()->GetSamplerState());
		irradiancePS->CopyAllBufferData();
		irradiancePS->SetShader();

        context->RSSetState(ge->getMaterial()->GetRastState());
		context->OMSetDepthStencilState(ge->getMaterial()->GetDepthStencilState(), 0);

		context->DrawIndexed(ge->getMesh()->GetIndexCount(), 0, 0);
	
	}




	// Reset the render states we've changed
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);  
}

void CaptureIrradiance::CreateCaptureMatrices()
{
	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		0.5f * 3.1415926535f,		// Field of View Angle
		1.0f,		// Aspect ratio
		0.1f,						// Near clip plane distance
		10.0f);					// Far clip plane distance
	XMStoreFloat4x4(&captureProjection, DirectX::XMMatrixTranspose(P));

	XMFLOAT3 eyePos = XMFLOAT3(0, 0, 0);
	XMVECTOR camPos = XMLoadFloat3(&eyePos);
	//XMVECTOR camPos = XMVectorSet(0, 0, 0, 0);
	XMVECTOR up[6];
	up[0] = XMVectorSet(0, 1, 0, 0);
	up[1] = XMVectorSet(0, 1, 0, 0);
	up[2] = XMVectorSet(0, 0, -1, 0);
	up[3] = XMVectorSet(0, 0, 1, 0); 
	up[4] = XMVectorSet(0, 1, 0, 0);
	up[5] = XMVectorSet(0, 1, 0, 0);

	XMVECTOR camDir[6];
	camDir[0] = XMVectorSet(1, 0, 0, 0);
	camDir[1] = XMVectorSet(-1, 0, 0, 0);
	camDir[2] = XMVectorSet(0, 1, 0, 0);
	camDir[3] = XMVectorSet(0, -1, 0, 0); 
	camDir[4] = XMVectorSet(0, 0, 1, 0);
	camDir[5] = XMVectorSet(0, 0, -1, 0);

	for (int i = 0; i < 6; i++) {
		XMMATRIX V = DirectX::XMMatrixLookToLH(
			camPos,     // The position of the "camera"
			camDir[i],     // Direction the camera is looking
			up[i]);     // "Up" direction in 3D space (prevents roll)
		XMStoreFloat4x4(&captureViews[i], DirectX::XMMatrixTranspose(V));
	}
}

