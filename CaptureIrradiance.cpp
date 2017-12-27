#include "CaptureIrradiance.h"
#include "DDSTextureLoader.h"
#include "DirectXTex.h"


CaptureIrradiance::CaptureIrradiance()
{
	irradianceMap = 0;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			capturedRTV[i][j] = 0;
		}
	}
	capturedSRV = 0;
	captureRTVResource = 0;
}

CaptureIrradiance::~CaptureIrradiance()
{
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			if(capturedRTV[i][j])
			capturedRTV[i][j]->Release();
		}
	}
	if (capturedSRV) capturedSRV->Release();
	if (irradianceMap) irradianceMap->Release();
}

bool CaptureIrradiance::Init(ID3D11Device * device , ID3D11DeviceContext * context, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	//texture description
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 5;
	textureDesc.ArraySize = 6; //cubemap
	//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;;

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
	for (int i = 0; i < 5; i++) {
		rtvDesc.Texture2DArray.MipSlice = i;
		for (int j = 0; j < 6; j++) {
			rtvDesc.Texture2DArray.FirstArraySlice = j;
			result = device->CreateRenderTargetView(irradianceMap, &rtvDesc, &capturedRTV[i][j]);
			if (FAILED(result))
			{
				return false;
			}
		}
	}

	//shader resource view description
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 4;
	srvDesc.TextureCube.MipLevels = -1;

	

	result = device->CreateShaderResourceView(irradianceMap, &srvDesc, &capturedSRV);
	//context->GenerateMips(capturedSRV);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC dsDesc;					
	dsDesc.Width = textureWidth;
	dsDesc.Height = textureHeight;
	dsDesc.Format = DXGI_FORMAT_D32_FLOAT;			
	dsDesc.ArraySize = 1;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.SampleDesc.Count = 1;					
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.MipLevels = 1;

	ID3D11Texture2D *depthStencilBuffer(NULL);
	device->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	
	device->CreateDepthStencilView(depthStencilBuffer, 0, &captureDSV);

	width = textureWidth;
	height = textureHeight;

	return true;
}

void CaptureIrradiance::SetRenderTarget(ID3D11RenderTargetView * capturedRTV, ID3D11DeviceContext * context)
{
	context->OMSetRenderTargets(1, &capturedRTV, captureDSV);
}

void CaptureIrradiance::ClearRenderTarget(ID3D11RenderTargetView * capturedRTV, ID3D11DeviceContext * context)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(capturedRTV, color);
	context->ClearDepthStencilView(
		captureDSV,
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

void CaptureIrradiance::RenderEnvironmentDiffuseMap(ID3D11DeviceContext * context, Entity * cubeForCapture)
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
	for (int i = 0; i <1; i++) {
		for (int j = 0; j < 6; j++) {
			ClearRenderTarget(capturedRTV[i][j], context);
			SetRenderTarget(capturedRTV[i][j], context);

			D3D11_VIEWPORT viewport = {};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = (float)width*std::pow(0.5, i);
			viewport.Height = (float)height*std::pow(0.5, i);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);

			irradianceVS = ge->getMaterial()->GetvertexShader();
			irradiancePS = ge->getMaterial()->GetpixelShader();

			irradianceVS->SetMatrix4x4("view", captureViews[j]);
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
	}
	// Reset the render states we've changed
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);	
}

void CaptureIrradiance::RenderPrefilteredMap(ID3D11DeviceContext * context, Entity * cubeForCapture)
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
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			ClearRenderTarget(capturedRTV[i][j], context);
			SetRenderTarget(capturedRTV[i][j], context);

			D3D11_VIEWPORT viewport = {};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = (float)width*std::pow(0.5, i);
			viewport.Height = (float)height*std::pow(0.5, i);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);

			irradianceVS = ge->getMaterial()->GetvertexShader();
			irradiancePS = ge->getMaterial()->GetpixelShader();

			irradianceVS->SetMatrix4x4("view", captureViews[j]);
			irradianceVS->SetMatrix4x4("projection", captureProjection);
			irradianceVS->CopyAllBufferData();
			irradianceVS->SetShader();

			irradiancePS->SetShaderResourceView("environmentMap", ge->getMaterial()->GetShaderResourceView());
			irradiancePS->SetSamplerState("basicSampler", ge->getMaterial()->GetSamplerState());
			irradiancePS->SetFloat("roughness", (float)i/ (float)5);
			irradiancePS->CopyAllBufferData();
			irradiancePS->SetShader();

			context->RSSetState(ge->getMaterial()->GetRastState());
			context->OMSetDepthStencilState(ge->getMaterial()->GetDepthStencilState(), 0);
			context->DrawIndexed(ge->getMesh()->GetIndexCount(), 0, 0);
		}
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

bool CaptureIrradiance::SaveEnvironmentDiffuseMap(ID3D11Device * device, ID3D11DeviceContext * context)
{
	ScratchImage image;
	HRESULT hr = CaptureTexture(device, context, irradianceMap, image);
	if (SUCCEEDED(hr))
	{
		hr = SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
			DDS_FLAGS_NONE, L"Assets/Textures/diffuseIrradianceMap.dds");
		if (FAILED(hr))
		{
			return false;
		}
	}
	return true;
}

bool CaptureIrradiance::EnvironmentDiffuseMapExists(ID3D11Device * device)
{
	HRESULT hr;
	hr = CreateDDSTextureFromFile(device, L"Assets/Textures/diffuseIrradianceMap.dds", 0, &capturedSRV);
	if (FAILED(hr)) return false;
	return true;
}

bool CaptureIrradiance::SavePrefilteredMap(ID3D11Device * device, ID3D11DeviceContext * context)
{
	ScratchImage image;
	HRESULT hr = CaptureTexture(device, context, irradianceMap, image);
	if (SUCCEEDED(hr))
	{
		hr = SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
			DDS_FLAGS_NONE, L"Assets/Textures/prefilteredMap.dds");
		if (FAILED(hr))
		{
			return false;
		}
	}
	return true;
}

bool CaptureIrradiance::PrefilteredMapExists(ID3D11Device * device)
{
	HRESULT hr;
	hr = CreateDDSTextureFromFile(device, L"Assets/Textures/prefilteredMap.dds", 0, &capturedSRV);
	if (FAILED(hr)) return false;
	return true;
}

