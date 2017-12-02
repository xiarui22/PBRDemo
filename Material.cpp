#include "Material.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include <fstream>

using namespace std;
using namespace DirectX;

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, ID3D11ShaderResourceView * ShaderResourceView, ID3D11SamplerState * SamplerState) {
	vertexShader = vertexShader;
	pixelShader = pixelShader;
	srv = ShaderResourceView;
	samplerState = SamplerState;
}

Material::Material(ID3D11Device * device, ID3D11DeviceContext * context, MaterialType type, const wchar_t * path, 
	const wchar_t *albedopath, const wchar_t *metallicpath, const wchar_t *roughnesspath, const wchar_t *aopath, const wchar_t * normalpath)
{
	samplerState = nullptr;
	srv = nullptr;

	this->type = type;

	if (type == kMaterialNormal) {
		SetTexture(device, context, path, normalpath);
	}
	else if (type == kMaterialCubemap) {
		SetupSkybox(device, context, path);
	}
	else if (type == kMaterialPBR) {
		SetPBRTexture(device, context, albedopath, metallicpath, roughnesspath, aopath, normalpath);
	}
}

void Material::SetTexture(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* path, const wchar_t* normalpath = nullptr)
{
	D3D11_SAMPLER_DESC sampleDes = {};
	sampleDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDes.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampleDes, &samplerState);

	DirectX::CreateWICTextureFromFile(device, context, path, 0, &srv);

	//if (normalpath) {
	//	ID3D11ShaderResourceView *temp = static_cast<ID3D11ShaderResourceView*>(normalSRV);
	//	DirectX::CreateWICTextureFromFile(device, context, path, 0, &temp);
	//}
}

void Material::SetupSkybox(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* path)
{
	D3D11_SAMPLER_DESC sampleDes = {};
	sampleDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDes.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampleDes, &samplerState);

	HRESULT hr;
	hr = DirectX::CreateDDSTextureFromFile(device, path, 0, &srv);

	if (FAILED(hr)) return;

	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &rsSky);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &dsSky);
}

void Material::SetPBRTexture(ID3D11Device * device, ID3D11DeviceContext * context, 
	const wchar_t *albedopath, const wchar_t *metallicpath, const wchar_t *roughnesspath, const wchar_t *aopath, const wchar_t * normalpath)
{
	D3D11_SAMPLER_DESC sampleDes = {};
	sampleDes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDes.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDes.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampleDes, &samplerState);

	HRESULT hr;
	hr = CreateWICTextureFromFile(device, context, albedopath, 0, &albedoSrv);
	hr = CreateWICTextureFromFile(device, context, metallicpath, 0, &metallicSrv);
	hr = CreateWICTextureFromFile(device, context, roughnesspath, 0, &roughnessSrv);
	hr = CreateWICTextureFromFile(device, context, aopath, 0, &aoSrv);
	hr = CreateWICTextureFromFile(device, context, normalpath, 0, &normalSrv);

	if (FAILED(hr)) return;

}

void Material::LoadVertexShaders(ID3D11Device * device, ID3D11DeviceContext * context, const wchar_t * name)
{
	vertexShader = new SimpleVertexShader(device, context);
	wstring path = L"Debug/" + (wstring)name + L".cso";
	const wchar_t *src = path.c_str();
	vertexShader->LoadShaderFile(src);
}

void Material::LoadPixelShaders(ID3D11Device *device, ID3D11DeviceContext * context, const wchar_t * name)
{
	pixelShader = new SimplePixelShader(device, context);
	wstring path = L"Debug/" + (wstring)name + L".cso";
	const wchar_t *src = path.c_str();
	pixelShader->LoadShaderFile(src);
}


void Material::SetVertexShaderMatrix(XMFLOAT4X4 worldm, XMFLOAT4X4 viewm, XMFLOAT4X4 projectionm)
{
	vertexShader->SetMatrix4x4("world", worldm);
	vertexShader->SetMatrix4x4("view", viewm);
	vertexShader->SetMatrix4x4("projection", projectionm);

	vertexShader->CopyAllBufferData();

	vertexShader->SetShader();
}

void Material::SetPixelShaderSrv()
{
	pixelShader->SetShaderResourceView("diffuseTexture", srv);
	pixelShader->SetSamplerState("basicSampler", samplerState);
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
}

void Material::SetSkyPixelShaderSrv()
{
	pixelShader->SetShaderResourceView("environmentMap", srv);
	pixelShader->SetSamplerState("basicSampler", samplerState);
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
}

void Material::SetPBRPixelShaderSrv()
{
	pixelShader->SetShaderResourceView("albedoMap", albedoSrv);
	pixelShader->SetShaderResourceView("metallicMap", metallicSrv);
	pixelShader->SetShaderResourceView("roughnessMap", roughnessSrv);
	pixelShader->SetShaderResourceView("aoMap", aoSrv);
	pixelShader->SetShaderResourceView("normalMap",normalSrv);
	pixelShader->SetShaderResourceView("irradianceMap", environmentDiffuseSrv);
    pixelShader->SetSamplerState("basicSampler", samplerState);
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
}

void Material::SetEnvironmentDiffuseSrvForPBR(ID3D11ShaderResourceView * environmentDiffuse)
{
	environmentDiffuseSrv = environmentDiffuse;
}

SimpleVertexShader * Material::GetvertexShader() {
	return vertexShader;
}

SimplePixelShader * Material::GetpixelShader() {
	return pixelShader;
}
ID3D11ShaderResourceView * Material::GetShaderResourceView() {
	return srv;
}
ID3D11SamplerState * Material::GetSamplerState() {
	return samplerState;
}

ID3D11RasterizerState* Material::GetRastState()
{
	return rsSky;
}

ID3D11DepthStencilState* Material::GetDepthStencilState()
{
	return dsSky;
}

void Material::SetSamplerState(ID3D11SamplerState *SamplerState) {
	samplerState = SamplerState;
}
void Material::SetShaderResource(ID3D11ShaderResourceView * ShaderResourceView) {
	srv = ShaderResourceView;
}
Material::~Material()
{
	delete vertexShader;
	delete pixelShader;
}
