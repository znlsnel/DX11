#include "Common.hlsli" // ���̴������� include ��� ����

// �����ڷ�
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

TextureCube specularIBLTex : register(t0);
TextureCube irradianceIBLTex : register(t1);
Texture2D brdfTex : register(t2);
Texture2D albedoTex : register(t3);
Texture2D normalTex : register(t4);
Texture2D aoTex : register(t5);
Texture2D metallicTex : register(t6);
Texture2D roughnessTex : register(t7);
Texture2D emissiveTex : register(t8);

SamplerState linearSampler : register(s0);
SamplerState clampSampler : register(s1);

static const float3 Fdielectric = 0.04; // ��ݼ�(Dielectric) ������ F0

cbuffer BasicPixelConstData : register(b0)
{
    float3 eyeWorld;
    float mipmapLevel;
    Material material;
    Light light[MAX_LIGHTS];
    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float exposure;
    float gamma;
    float3 dummy;
};

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
    //return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

float3 GetNormal(PixelShaderInput input)
{
    float3 normalWorld = input.normalWorld;
    
    if (useNormalMap) // NormalWorld�� ��ü
    {
        float3 normal = normalTex.SampleLevel(linearSampler, input.texcoord, 0.0).rgb;
        normal = 2.0 * normal - 1.0; // ���� ���� [-1.0, 1.0]

        // OpenGL �� ��ָ��� ��쿡�� y ������ �������ݴϴ�.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix�� float4x4, ���⼭�� ���� ��ȯ���̶� 3x3 ���
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                  float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 irradiance = irradianceIBLTex.Sample(linearSampler, normalWorld).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    float2 specularBRDF = brdfTex.Sample(clampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness)).rg;
    float3 specularIrradiance = specularIBLTex.SampleLevel(linearSampler, reflect(-pixelToEye, normalWorld), 
                                                            roughness * 5.0f).rgb;
    const float3 Fdielectric = 0.04; // ��ݼ�(Dielectric) ������ F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
                            float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;

    return alphaSq / (3.141592 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float SchlickG1(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

PixelShaderOutput main(PixelShaderInput input)
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);

    float3 albedo = useAlbedoMap ? albedoTex.Sample(linearSampler, input.texcoord).rgb 
                                 : material.albedo;

    float ao = useAOMap ? aoTex.SampleLevel(linearSampler, input.texcoord, 0.0).r + 0.2: 1.0;
    
    float metallic = useMetallicMap ? metallicTex.Sample(linearSampler, input.texcoord).b 
                                    : material.metallic;
    float roughness = useRoughnessMap ? roughnessTex.Sample(linearSampler, input.texcoord).g
                                      : material.roughness;
    float3 emission = useEmissiveMap ? emissiveTex.Sample(linearSampler, input.texcoord).rgb 
                                     : float3(0, 0, 0);
    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao,
                                                  metallic, roughness);

    float3 directLighting = float3(0, 0, 0);

    // ����Ʈ ����Ʈ�� ���� ����
    [unroll]
    for (int i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        float3 lightVec = light[i].position - input.posWorld;
        float3 halfway = normalize(pixelToEye + lightVec);
        
        float NdotI = max(0.0, dot(normalWorld, lightVec));
        float NdotH = max(0.0, dot(normalWorld, halfway));
        float NdotO = max(0.0, dot(normalWorld, pixelToEye));
        
        const float3 Fdielectric = 0.04; // ��ݼ�(Dielectric) ������ F0
        float3 F0 = lerp(Fdielectric, albedo, metallic);
        float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
        float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
        float3 diffuseBRDF = kd * albedo;

        float D = NdfGGX(NdotH, roughness);
        float3 G = SchlickGGX(NdotI, NdotO, roughness);
        float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

        float3 radiance = light[i].radiance * saturate((light[i].fallOffEnd - length(lightVec)) / (light[i].fallOffEnd - light[i].fallOffStart));

        directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
        //directLighting += (specularBRDF) * radiance * NdotI;
    }
    
    PixelShaderOutput output;
    output.pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
    
    return output;
}
