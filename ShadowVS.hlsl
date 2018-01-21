
cbuffer externalData : register(b0)
{
	matrix world;
	matrix lightView;
	matrix lightProjection;
};


struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float3 normal       : NORMAL;
	float2 uv           : TEXCOORD;
	float3 tangent		: TANGENT;
};


struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
};


VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	
	matrix worldViewProjLightSpace = mul(mul(world, lightView), lightProjection);

	output.position = mul(float4(input.position, 1.0f), worldViewProjLightSpace);
	return output;
}