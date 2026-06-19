#include "Includes/lighting.hlsli"
#include "Includes/platform.hlsli"

struct Camera
{
	float4x4 View;
	float4x4 Projection;
	float4x4 Model;
	float4 Position;
};
ConstantBuffer<Camera> uCamera : register(b1, space0);

struct Light
{
	float3 Position;
	float4 Color;
};

struct LightData
{
	Light Lights[4];
};
ConstantBuffer<LightData> uLights : register(b2, space0);

Texture2D uTextures[] : register(t0, space1);
SamplerState uSampler : register(s0, space0);

struct Input
{
	float3 WorldPos : POSITION0;
	float3 Normal : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

struct PushConstants
{
	float4x4 Transform;
	uint InstanceOffset;
	int DiffuseMapIndex;
	int NormalMapIndex;
	int RoughMetMapIndex;
	float Metallic;
	float Roughness;
};
PUSH_CONSTANTS
ConstantBuffer<PushConstants> uPC : register(b0, space1);

float4 Main(Input input) : SV_Target
{
	float3 N = normalize(input.Normal);
	float3 V = normalize(uCamera.Position.xyz - input.WorldPos);
	float3 color;
	
	if (uPC.DiffuseMapIndex > -1)
		color = uTextures[NonUniformResourceIndex(uPC.DiffuseMapIndex)].Sample(uSampler, input.TexCoord).rgb;
	else
		color = float3(1.0, 1.0, 1.0);
	
	// Specular contribution
	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 4; i++)
	{
		Light l = uLights.Lights[i];
		float3 L = normalize(l.Color.xyz - input.WorldPos);
		Lo += BRDF(color, L, V, N, uPC.Metallic, uPC.Roughness);
	}
	
	// Ambient
	float3 outColor = color * 0.02;
	outColor += Lo;
	
	// Gamma correction
	outColor = pow(outColor, float3(0.4545, 0.4545, 0.4545));
	
	return float4(outColor, 1.0);
}