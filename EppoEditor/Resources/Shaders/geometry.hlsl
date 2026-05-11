struct VSInput
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
};

struct PushConstants
{
    float3 MeshPosition;
};
[[vk::push_constant]] PushConstants uPC;

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

struct VSOutput
{
    float4 Position : SV_Position;
    float3 WorldPos : POSITION0;
    float3 Normal : NORMAL0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.WorldPos = mul(uCamera.Model, float4(input.Position, 1.0)).xyz + uPC.MeshPosition;
    output.Normal = mul((float3x3) uCamera.Model, input.Normal);
    output.Position = mul(uCamera.Projection, mul(uCamera.View, float4(output.WorldPos, 1.0)));

    return output;
}

float4 PSMain(VSOutput input)
{
    
}