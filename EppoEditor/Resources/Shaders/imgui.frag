struct Input
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D uTexture : register(t0);
sampler uSampler : register(s0);

float4 Main(Input input) : SV_TARGET
{
    float4 sample = uTexture.Sample(uSampler, input.UV);
    return input.Color * sample;
}