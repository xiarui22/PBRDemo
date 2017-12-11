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

// Generate biased sample vectors "randomly"
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

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness) {
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a) - 1.0)*Xi.y);
	float sinTheta = sqrt(1 - cosTheta*cosTheta);

	// From spherical to cartesian
	float3 H;
	H.x = cos(phi)*sinTheta;
	H.y = sin(phi)*sinTheta;
	H.z = cosTheta;

	// From tangent to world-space
	float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent*H.x + bitangent*H.y + up*H.z;
	return normalize(sampleVec);
}

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 N = normalize(input.localPos);
	float3 R = N;
	float3 V = R;

	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0, 0, 0);
	for (uint i = 0u; i < SAMPLE_COUNT; i++) {
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0*dot(V, H)*H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0) {
			prefilteredColor += environmentMap.Sample(basicSampler, L).rgb*NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColor = prefilteredColor / totalWeight;

	float4 FragColor = float4(prefilteredColor, 1.0);
	return FragColor;

}