// External texture-related data
TextureCube environmentMap	: register(t0);
SamplerState basicSampler	: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 localPos		: LOCALPOS;
};

cbuffer externalData : register(b0)
{
	float roughness;
};

static float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

//float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness) {
//	float a = roughness*roughness;
//	float phi = 
//}

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	
	//float4 FragColor = float4(irradiance, 1.0);
	float4 FragColor = float4(1,1,1, 1.0);
	return FragColor;

}