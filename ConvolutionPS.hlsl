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

//static float PI = 3.14159265359;

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	//float3 normal = normalize(input.position);
	//float3 irradiance = float3(0.0,0.0,0.0);

	////convolution
	//float3 up = float3(0, 1, 0);
	//float3 right = cross(up, normal);
	//up = cross(normal, right);

	//float sampleDelta = 0.025;
	//float nrSamples = 0.0;
	//for (float phi = 0; phi < 2 * PI; phi += sampleDelta) {
	//	for (float theta = 0; theta < 0.5*PI; theta += sampleDelta) {
	//		float3 tangentSample = float3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
	//		float3 sampleVec = tangentSample.x*right + tangentSample.y*up + tangentSample.z*normal;
	//		irradiance += environmentMap.Sample(basicSampler, sampleVec).rgb*cos(theta)*sin(theta);
	//		nrSamples++;
	//	}
	//}

	//irradiance = PI*irradiance*(1.0 / float(nrSamples));
	//irradiance = environmentMap.Sample(basicSampler, input.uvw).xyz;
	//float4 FragColor = float4(irradiance, 1.0);
	//float4 FragColor = float4(0,0,1, 1);
	return environmentMap.Sample(basicSampler, input.uvw);
	//return FragColor;
}