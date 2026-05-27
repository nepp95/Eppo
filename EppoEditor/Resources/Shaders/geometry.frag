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

struct LightData
{
	float4 Lights[4];
};
ConstantBuffer<LightData> uLights : register(b2, space0);

struct Input
{
	float3 WorldPos : POSITION0;
	float3 Normal : NORMAL0;
};

struct PushConstants
{
	float4x4 Transform;
	float3 MeshPosition;
	float Metallic;
	float3 Color;
	float Roughness;
};
PUSH_CONSTANTS
ConstantBuffer<PushConstants> uPC : register(b0, space1);

float4 Main(Input input) : SV_Target
{
	float3 N = normalize(input.Normal);
	float3 V = normalize(uCamera.Position.xyz - input.WorldPos);
	
	// Specular contribution
	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 4; i++)
	{
		float3 L = normalize(uLights.Lights[i].xyz - input.WorldPos);
		Lo += BRDF(uPC.Color, L, V, N, uPC.Metallic, uPC.Roughness);
	}
	
	// Ambient
	float3 color = uPC.Color * 0.02;
	color += Lo;
	
	// Gamma correction
	color = pow(color, float3(0.4545, 0.4545, 0.4545));

	return float4(color, 1.0);
}