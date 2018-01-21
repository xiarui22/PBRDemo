#include "ShadowMapRenderer.h"

using namespace std;

ShadowMapRenderer::ShadowMapRenderer()
{
}


ShadowMapRenderer::~ShadowMapRenderer()
{
}

bool ShadowMapRenderer::Init(ID3D11Device * device, ID3D11DeviceContext * context, int textureWidth, int textureHeight)
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
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags =  D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//create the render target texture
	result = device->CreateTexture2D(&textureDesc, 0, &depthMap);
	if (FAILED(result))
	{
		return false;
	}


	//shader resource view description
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	result = device->CreateShaderResourceView(depthMap, &srvDesc, &capturedSRV);
	//context->GenerateMips(capturedSRV);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthMap, &shadowDSDesc, &captureDSV);

	width = textureWidth;
	height = textureHeight;

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;

	D3D11_SAMPLER_DESC sampleDesClamp = {};
	sampleDesClamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesClamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesClamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesClamp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	//device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
	device->CreateSamplerState(&sampleDesClamp, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_FRONT;   // remove offset of shadow?
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);


	return true;
}

void ShadowMapRenderer::SetRenderTarget(ID3D11DeviceContext * context)
{
	//context->OMSetRenderTargets(1, &capturedRTV, captureDSV);
	context->OMSetRenderTargets(0, 0, captureDSV);
}

void ShadowMapRenderer::ClearRenderTarget(ID3D11DeviceContext * context)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	//context->ClearRenderTargetView(capturedRTV, color);
	context->ClearDepthStencilView(
		captureDSV,
		D3D11_CLEAR_DEPTH,
		1.0f,
		0);
}

ID3D11ShaderResourceView * ShadowMapRenderer::GetShaderResourceView()
{
	return capturedSRV;
}

ID3D11Texture2D * ShadowMapRenderer::GetTexture2d()
{
	return depthMap;
}

ID3D11SamplerState * ShadowMapRenderer::GetShadowSampler()
{
	return shadowSampler;
}



void ShadowMapRenderer::RenderDepthMap(ID3D11DeviceContext * context, Scene * scene)
{
	XMFLOAT3 test = scene->pointLight2.pointLightPosition;
	test = XMFLOAT3(0, 5, -2);
	CreateMatrices(test);
	//CreateMatrices(XMFLOAT3(0,0,0));

	SetRenderTarget(context);
	ClearRenderTarget(context);
	context->RSSetState(shadowRasterizer);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	shadowMapVS = scene->shadowMap->GetvertexShader();
	shadowMapVS->SetShader();
	shadowMapVS->SetMatrix4x4("lightView", lightViewMatrix);
	shadowMapVS->SetMatrix4x4("lightProjection", lightProjectionMatrix);
	context->PSSetShader(0, 0, 0); // turn off pixel shader
	for (int i = 0; i <2; i++) {

		scene->quads[i]->setWorld(scene->quads[i]->getScale(), scene->quads[i]->getRotate(), scene->quads[i]->getTranslation());

		shadowMapVS->SetMatrix4x4("world", scene->quads[i]->getWorld());
		
		shadowMapVS->CopyAllBufferData();

		ID3D11Buffer* vertexBuffer = scene->quads[i]->getMesh()->GetVertexBuffer();

		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

		context->IASetIndexBuffer(scene->quads[i]->getMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(
			scene->quads[i]->getMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	}


	for (int i = 0; i < 5; i++) {
		for (int j = 0; j <5; j++) {

			scene->spheres[i][j]->setWorld(scene->spheres[i][j]->getScale(), scene->spheres[i][j]->getRotate(), scene->spheres[i][j]->getTranslation());

			//set light

			shadowMapVS->SetMatrix4x4("world", scene->spheres[i][j]->getWorld());
	
			shadowMapVS->CopyAllBufferData();
		
			ID3D11Buffer* vertexBuffer = scene->spheres[i][j]->getMesh()->GetVertexBuffer();

			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

			context->IASetIndexBuffer(scene->spheres[i][j]->getMesh()->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

			context->DrawIndexed(
				scene->spheres[i][j]->getMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
				0,     // Offset to the first index we want to use
				0);    // Offset to add to each index when looking up vertices
		}
	}
}


void ShadowMapRenderer::CreateMatrices(DirectX::XMFLOAT3 lightPos)
{
	lightPosition = lightPos;

	//XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
	//	0.5f * 3.1415926535f,		// Field of View Angle
	//	1.0f,		// Aspect ratio
	//	0.1f,						// Near clip plane distance
	//	100.0f);					// Far clip plane distance
	//XMStoreFloat4x4(&lightProjectionMatrix, DirectX::XMMatrixTranspose(P));
	// Shadow proj matrix
	XMMATRIX P = XMMatrixOrthographicLH(
		10.0f,		// Ortho width
		10.0f,		// Ortho height
		0.1f,		// Near plane
		100.0f);	// Far plane
	XMStoreFloat4x4(&lightProjectionMatrix, XMMatrixTranspose(P));

	/*XMMATRIX P = XMMatrixOrthographicLH((float)1, (float)1, 1.0f, 1000.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, XMMatrixTranspose(P));*/

	XMFLOAT3 eyePos = lightPosition;
	XMVECTOR camPos = XMLoadFloat3(&eyePos);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR camDes = XMVectorSet(0,0,0, 0);
	XMVECTOR camDir = XMVectorSet(0, -1, 1, 0);
	//XMMATRIX V = DirectX::XMMatrixLookAtLH(
	XMMATRIX V = DirectX::XMMatrixLookToLH(
		camPos,     // The position of the "camera"
	//	camDes,     // Direction the camera is looking
		camDir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&lightViewMatrix, XMMatrixTranspose(V));

}
