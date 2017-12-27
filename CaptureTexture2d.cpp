#include "CaptureTexture2d.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DirectXTex.h"

using namespace DirectX;

CaptureTexture2d::CaptureTexture2d()
{
}


CaptureTexture2d::~CaptureTexture2d()
{
}

bool CaptureTexture2d::Init(ID3D11Device * device, ID3D11DeviceContext * context, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	//texture description
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1; 
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//create the render target texture
	result = device->CreateTexture2D(&textureDesc, 0, &texture2d);
	if (FAILED(result))
	{
		return false;
	}

	//render target view description
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2DArray.MipSlice = 0;
	result = device->CreateRenderTargetView(texture2d, &rtvDesc, &capturedRTV);
	if (FAILED(result))
	{
		return false;
	}
	

	//shader resource view description
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;



	result = device->CreateShaderResourceView(texture2d, &srvDesc, &capturedSRV);
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

void CaptureTexture2d::SetRenderTarget(ID3D11DeviceContext * context)
{
	context->OMSetRenderTargets(1, &capturedRTV, captureDSV);
}

void CaptureTexture2d::ClearRenderTarget(ID3D11DeviceContext * context)
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

ID3D11ShaderResourceView * CaptureTexture2d::GetShaderResourceView()
{
	return capturedSRV;
}

ID3D11Texture2D * CaptureTexture2d::GetTexture2d()
{
	return texture2d;
}

void CaptureTexture2d::RenderBrdfLUT(ID3D11DeviceContext * context, Entity * plane)
{
	Entity * ge = plane;

	ID3D11Buffer* vb = ge->getMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = ge->getMesh()->GetIndexBuffer();
	CreateCaptureMatrices();
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	
	ClearRenderTarget(context);
	SetRenderTarget(context);

    D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

    texture2dVS = ge->getMaterial()->GetvertexShader();
	texture2dPS = ge->getMaterial()->GetpixelShader();

	texture2dVS->SetMatrix4x4("view", captureView);
	texture2dVS->SetMatrix4x4("projection", captureProjection);
	texture2dVS->CopyAllBufferData();
	texture2dVS->SetShader();

	texture2dPS->CopyAllBufferData();
	texture2dPS->SetShader();

	context->DrawIndexed(ge->getMesh()->GetIndexCount(), 0, 0);

}

void CaptureTexture2d::CreateCaptureMatrices()
{
	//XMMATRIX P = XMMatrixOrthographicLH((float)width, (float)height, -0.1f, 10.0f);
	XMMATRIX P = XMMatrixOrthographicLH((float)1, (float)1, 1.0f,1000.0f);
	XMStoreFloat4x4(&captureProjection, XMMatrixTranspose(P));

	//XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
	//	0.5f * 3.1415926535f,		// Field of View Angle
	//	//1.0f,		// Aspect ratio
	//	(float)1280 / 768,
	//	0.1f,						// Near clip plane distance
	//	10.0f);					// Far clip plane distance
	//XMStoreFloat4x4(&captureProjection, DirectX::XMMatrixTranspose(P));


	XMFLOAT3 eyePos = XMFLOAT3(0, 0, -5);
	XMVECTOR camPos = XMLoadFloat3(&eyePos);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR camDir = XMVectorSet(0, 0, 1, 0);

    XMMATRIX V = DirectX::XMMatrixLookToLH(
			camPos,     // The position of the "camera"
			camDir,     // Direction the camera is looking
			up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&captureView, XMMatrixTranspose(V));
}

bool CaptureTexture2d::SaveBrdfLUT(ID3D11Device * device, ID3D11DeviceContext * context)
{
	ScratchImage image;
	HRESULT hr = CaptureTexture(device, context, texture2d, image);
	const Image* img = image.GetImage(0, 0, 0);
	assert(img);
	
	if (SUCCEEDED(hr))
	{
		hr = SaveToWICFile(*img, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_JPEG), L"Assets/Textures/BrdfLUT.png");
		if (FAILED(hr))
		{
			return false;
		}
	}
	return true;
}

bool CaptureTexture2d::BrdfLUTExists(ID3D11Device * device, ID3D11DeviceContext * context)
{
	HRESULT hr;
	hr = CreateWICTextureFromFile(device, context, L"Assets/Textures/BrdfLUT.png", 0, &capturedSRV);
	if (FAILED(hr)) return false;
	return true;
}


