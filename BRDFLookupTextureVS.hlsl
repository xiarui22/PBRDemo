
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 localPos		: LOCALPOS;
	float i             : PADDING;
	float2 uv           : TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;
	output.localPos = input.position;

	matrix viewProj = mul(view, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);
	output.uv = input.uv;

	return output;
}