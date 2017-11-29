
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	//float4 color		: COLOR;
	float3 normal       : NORMAL;
	float3 worldPos		: WORLDPOS;
	float2 uv           : TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------



struct DirectionalLight {
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

struct PointLight {
	float4 pointLightColor;
	float3 pointLightPosition;
	float i;
	float3 cameraPosition;
};

cbuffer externalData : register(b0)
{
	DirectionalLight dl;
	DirectionalLight dl0;
	PointLight pl;
};

Texture2D diffuseTexture  : register(t0);
SamplerState basicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	input.normal = normalize(input.normal);
//directional light 
float lightAmountDL = saturate(dot(input.normal, normalize(-dl.Direction)));
float4 dlight = dl.DiffuseColor* lightAmountDL*surfaceColor + dl.AmbientColor;

float lightAmountDL0 = saturate(dot(input.normal, normalize(-dl0.Direction)));
float4 dlight0 = dl0.DiffuseColor* lightAmountDL0*surfaceColor + dl0.AmbientColor;

//point light
float3 dirToPointLight = normalize(pl.pointLightPosition - input.worldPos);
float lightAmountPL = saturate(dot(input.normal, dirToPointLight));
float4 plight = pl.pointLightColor * lightAmountPL*surfaceColor;
// Specular highlight for point light
float3 toCamera = normalize(pl.cameraPosition - input.worldPos);
float3 refl = reflect(-dirToPointLight, input.normal);
float specular = pow(saturate(dot(refl, toCamera)), 8);

return  dlight + dlight0+ plight;
	//return float4(1,1,1,1);

}