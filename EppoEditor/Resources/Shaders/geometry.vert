#include "Includes/platform.hlsli"

struct Input
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

struct PushConstants
{
	float4x4 Transform;
};
PUSH_CONSTANTS
ConstantBuffer<PushConstants> uPC : register(b0, space0);

struct Camera
{
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	float3 Position;
};
ConstantBuffer<Camera> uCamera : register(b1, space0);

struct Output
{
	float4 Position : SV_Position;
	float3 WorldPos : POSITION0;
	float3 Normal : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

Output Main(Input input)
{
	Output output;
	output.WorldPos = mul(uPC.Transform, float4(input.Position, 1.0)).xyz;
	output.Normal = mul((float3x3) uPC.Transform, input.Normal);
	output.Position = mul(uCamera.Projection, mul(uCamera.View, float4(output.WorldPos, 1.0)));
	output.TexCoord = input.TexCoord;

	return output;
}