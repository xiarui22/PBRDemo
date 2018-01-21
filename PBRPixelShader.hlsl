
struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal       : NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: WORLDPOS;
	float2 uv           : TEXCOORD;
	float4 posForShadow : POSITION1;
};

struct PointLight {
	float4 pointLightColor;
	float3 pointLightPosition;
	float i;
};

cbuffer externalData : register(b0)
{
	//lights
	PointLight pl0;
	PointLight pl1;
	PointLight pl2;
	PointLight pl3;
	//camera position
	float3 camPos;
	//parameters
	float metallicP;
	float roughnessP;
};


Texture2D albedoMap: register(t0);
Texture2D metallicMap: register(t1);
Texture2D roughnessMap: register(t2);
Texture2D aoMap: register(t3);
Texture2D normalMap: register(t4);
TextureCube irradianceMap: register(t5);
TextureCube prefilterMap: register(t6);
Texture2D brdfLUT: register(t7);
Texture2D shadowMap		: register(t8);

SamplerState basicSampler : register(s0);
SamplerState samplerForLUT : register(s1);
//SamplerComparisonState shadowSampler : register(s2);
SamplerState shadowSampler : register(s2);
static float PI = 3.14159265359f;

float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH*NdotH;

	float nom = a2;
	float denom = NdotH2*(a2 - 1.0) + 1.0;
	denom = PI * denom*denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;  //direct lighting
	float k = r*r / 8.0;

	float nom = NdotV;
	float denom = NdotV*(1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0);
	float NdotL = max(dot(N, L), 0);

	float ggx1 = GeometrySchlickGGX(NdotV,roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1*ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0)*pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 getNormalFromNormalMap(VertexToPixel input) {
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	//---------------------------------------------------------------------------------------------
	//Normal Mapping

	//Read and unpack the normal from the map
	float3 normalFromMap = normalMap.Sample(basicSampler, input.uv).xyz * 2 - 1;

	//Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));
	return input.normal;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 albedo;
	float metallic;
	float roughness;
	float ao;
	float3 normal;

	//albedo = pow(albedoMap.Sample(basicSampler, input.uv).rgb, 2.2);
	albedo = float4(1, 1, 1, 1);
	//input.normal = getNormalFromNormalMap(input);
	input.normal = normalize(input.normal);
	//metallic = metallicMap.Sample(basicSampler, input.uv).r + metallicP;
	//roughness = roughnessMap.Sample(basicSampler, input.uv).r + roughnessP;
	//ao = aoMap.Sample(basicSampler, input.uv).r;
	metallic = metallicP;
	roughness = roughnessP;
	ao = 1;

	PointLight pointLight[4];
    pointLight[0] = pl0;
	pointLight[1] = pl1;
	pointLight[2] = pl2;
	pointLight[3] = pl3;
	float3 N = normalize(input.normal);
	float3 V = normalize(camPos - input.worldPos);
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

	//relfectance equation
	float3 Lo = float3(0, 0, 0);
	for (int i = 0; i < 4; i++) {
		//calculate per-light radiance
		float3 L = normalize(pointLight[i].pointLightPosition - input.worldPos);
		float3 H = normalize(L + V);
		float distance = length(pointLight[i].pointLightPosition - input.worldPos);
		float attenuation = 1 / (distance*distance);
		float3 radiance = pointLight[i].pointLightColor*attenuation;

		//cook-torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float3 F = fresnelSchlick(max(dot(N, V),0), F0);
		float G = GeometrySmith(N, V, L, roughness);

		float3 kS = F;
		float3 kD = float3(1, 1, 1) - kS;
		kD *= 1 - metallic;

		float3 nominator = NDF * F * G;
		float denominator = 4 * max(dot(V, N),0)*max(dot(L, N),0) + 0.001; //avoid divide 0
		float3 specular = nominator / denominator;

		float NdotL = max(dot(N, L),0);
		Lo += (kD*albedo / PI + specular)*radiance*NdotL;
	}

	// Add environment diffuse 
	float3 F = fresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);
	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	float3 irradiance = irradianceMap.SampleLevel(basicSampler, N, 0).rgb;
	float3 diffuse = irradiance * albedo;

	float3 R = reflect(-V, N);
	const float MAX_REFLECTION_LOD = 4.0;
	float3 prefilteredColor = prefilterMap.SampleLevel(basicSampler, R, roughness * MAX_REFLECTION_LOD).rgb;

	float2 envBRDF = brdfLUT.SampleLevel(samplerForLUT, float2(max(dot(N, V), 0), roughness),0).rg;
	float3 specular = prefilteredColor * (F*envBRDF.x + envBRDF.y);

	
	float3 ambient = (kD * diffuse + specular) * ao;
	
	float3 color = float3(0,0,0);

	// Shadow mapping
	// Percentage closer filtering
	
	float width, height;
	shadowMap.GetDimensions(width, height);
    float2 textureSize = float2(width,height);
	float2 texelSize = 1 / textureSize;
	float bias = 0.001f;         //  Set the bias value for fixing the floating point precision issues.
	float3 projCoords = input.posForShadow.xyz / input.posForShadow.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	projCoords.y = 1 - projCoords.y;


	if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y)) {
		
		float currentDepth = projCoords.z;
		currentDepth -= bias;
		float shadow = 0;
		for (int i = -3; i < 4; i++) {
			for (int j = -3; j < 4; j++) {
               float cloestDepth = shadowMap.Sample(shadowSampler, projCoords.xy + float2(i,j) * texelSize).r;
			   shadow += currentDepth > cloestDepth  ? 1 : 0;
			}
		}
		shadow /= 27;
		color = (ambient + Lo) * (1 - shadow);
	}
	else {
		//color = ambient + Lo;
	}
	

	// HDR
	color = color / (color + float3(1, 1, 1));
	// Gamma correction
	color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	float4 FragColor = float4(color, 1.0);
	return FragColor;
}