#include "Includes/platform.hlsli"

struct Input
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
};

struct PushConstants
{
	float3 MeshPosition;
};
PUSH_CONSTANTS
ConstantBuffer<PushConstants> uPC : register(b0, space1);

struct Camera
{
	float4x4 View;
	float4x4 Projection;
	float4x4 Model;
	float4 Position;
};
ConstantBuffer<Camera> uCamera : register(b0, space0);

struct Output
{
	float4 Position : SV_Position;
	float3 WorldPos : POSITION0;
	float3 Normal : NORMAL0;
};

Output Main(Input input)
{
	Output output;
	output.WorldPos = mul(uCamera.Model, float4(input.Position, 1.0)).xyz + uPC.MeshPosition;
	output.Normal = mul((float3x3) uCamera.Model, input.Normal);
	output.Position = mul(uCamera.Projection, mul(uCamera.View, float4(output.WorldPos, 1.0)));

	return output;
}