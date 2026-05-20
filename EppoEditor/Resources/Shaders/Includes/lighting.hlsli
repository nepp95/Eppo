#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

static const float PI = 3.14159265359;

// Normal distribution GGX
float DGGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denominator = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denominator * denominator);
}

// Geometry shadowing
float GSchlickSmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel
float3 FSchlick(float3 matColor, float cosTheta, float metallic)
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), matColor, metallic);
    float3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}

// BRDF
float3 BRDF(float3 matColor, float3 L, float3 V, float3 N, float metallic, float roughness)
{
    float3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float3 color = float3(0.0, 0.0, 0.0);
    
    if (dotNL > 0.0)
    {
        float rroughness = max(0.05, roughness);
        float D = DGGX(dotNH, roughness);
        float G = GSchlickSmithGGX(dotNL, dotNV, rroughness);
        float3 F = FSchlick(matColor, dotNV, metallic);
        float3 spec = D * F * G / (4.0 * dotNL * dotNV);
        color += spec * dotNL * lightColor;
    }
    
    return color;
}

#endif