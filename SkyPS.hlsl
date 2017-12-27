// External texture-related data
TextureCube environmentMap	: register(t0);
SamplerState basicSampler	: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;
};


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return environmentMap.SampleLevel(basicSampler, input.uvw, 1.2);
	//return environmentMap.Sample(basicSampler, input.uvw);
}