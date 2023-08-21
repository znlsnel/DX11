#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

// 참고자료
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

TextureCube specularIBLTex : register(t0);
TextureCube irradianceIBLTex : register(t1);
Texture2D brdfTex : register(t2);
Texture2D albedoTex : register(t3);
Texture2D normalTex : register(t4);
Texture2D aoTex : register(t5);
Texture2D metallicRoughnessTex : register(t6);
Texture2D emissiveTex : register(t7);
Texture2DArray shadowMap : register(t8); // <- 그림자

SamplerState linearSampler : register(s0);
SamplerState clampSampler : register(s1);
SamplerState borderSampler : register(s2);

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0

static const float lod = 2.0;

cbuffer BasicPixelConstData : register(b0)
{
    Material material;
    Light lights[MAX_LIGHTS];
    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float exposure;
    float gamma;
    float mipmapLevel;
    float2 dummy0;
};

cbuffer EyeViewProjConstData : register(b1)
{
    matrix viewProj;
    float3 eyeWorld;
    
    // Depth 전용 쉐이더를 별도로 만드는 것이 더 효율적이지만
    // 여기서는 구조를 단순하게 유지하기 위해 옵션 사용
    int depthPass; // 1이면 Depth 출력
};

cbuffer LightEyeViewProjConstData : register(b2)
{
    matrix lightViewProj;
    float3 lightEyeWorld;
    int lightDepthPass; // 최종 쉐이딩에는 미사용
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
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.SampleLevel(linearSampler, input.texcoord, lod).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
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
    float3 irradiance = irradianceIBLTex.SampleLevel(linearSampler, normalWorld, 0).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    float2 specularBRDF = brdfTex.SampleLevel(clampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float3 specularIrradiance = specularIBLTex.SampleLevel(linearSampler, reflect(-pixelToEye, normalWorld),
                                                            2 + roughness * 5.0f).rgb;
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
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
float3 DirectBRDF(float3 lightVec, float3 pixelToEye, float3 posWorld, float3 normalWorld, float3 albedo, float metallic, float roughness)
{
    float3 halfway = normalize(pixelToEye + lightVec);
        
    float NdotI = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotO = max(0.0, dot(normalWorld, pixelToEye));
        
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo;

    float D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);
    
    return (diffuseBRDF + specularBRDF) * NdotI;
}

float SampleShadowMap(float lightDistance, float2 uv, float id, float bias)
{
    // borderSampler 사용    
    float shadowDepth = shadowMap.SampleLevel(borderSampler, float3(uv, id), 0.0).r;    
    return lightDistance > shadowDepth + bias ? 0.0 : 1.0;
}

float ComputeShadowFactor(float3 posWorld, float id, float bias)
{
    float4 lightCoord = mul(float4(posWorld.xyz, 1.0), lightViewProj);

    if (lightCoord.w < 0.0)
    {
        return 1.0;
    }
    else
    {
        static const float dx = 1.0 / 1280 * 0.5, dy = 1.0 / 1280 * 0.5;

        float2 uv = lightCoord.xy / lightCoord.w;
        uv.y = -uv.y;
        uv = (uv + 1.0) * 0.5;

        float lightDistance = distance(lightEyeWorld, posWorld);
        
        float a = SampleShadowMap(lightDistance, uv + float2(-2 * dx, 2 * dy), id, bias);
        float b = SampleShadowMap(lightDistance, uv + float2(0, 2 * dy), id, bias);
        float c = SampleShadowMap(lightDistance, uv + float2(2 * dx, 2 * dy), id, bias);
        float d = SampleShadowMap(lightDistance, uv + float2(-2 * dx, 0), id, bias);
        float e = SampleShadowMap(lightDistance, uv, id, bias);
        float f = SampleShadowMap(lightDistance, uv + float2(2 * dx, 0), id, bias);
        float g = SampleShadowMap(lightDistance, uv + float2(-2 * dx, -2 * dy), id, bias);
        float h = SampleShadowMap(lightDistance, uv + float2(0, -2 * dy), id, bias);
        float i = SampleShadowMap(lightDistance, uv + float2(2 * dx, -2 * dy), id, bias);
        float j = SampleShadowMap(lightDistance, uv + float2(-dx, dy), id, bias);
        float k = SampleShadowMap(lightDistance, uv + float2(dx, dy), id, bias);
        float l = SampleShadowMap(lightDistance, uv + float2(-dx, -dy), id, bias);
        float m = SampleShadowMap(lightDistance, uv + float2(dx, -dy), id, bias);

        float shadowFactor = e * 0.125;
        shadowFactor += (a + c + g + i) * 0.03125;
        shadowFactor += (b + d + f + h) * 0.0625;
        shadowFactor += (j + k + l + m) * 0.125;
        
        return shadowFactor;
        //return e;
    }

/*  Luna D12 교재에서는 SampleCmpLevelZero()을 사용합니다.
    https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-samplecmp
    
    float dx = 1.0f / (float) width;
    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx), float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
                                                    shadowPosH.xy + offsets[i], depth).r;}
        return percentLit / 9.0f;
*/
}

PixelShaderOutput main(PixelShaderInput input)
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    // Depth 전용 쉐이더를 별도로 만드는 것이 더 효율적이지만
    // 여기서는 구조를 단순하게 유지하기 위해 옵션 사용
    if (depthPass)
    {
        PixelShaderOutput output;
        output.pixelColor = float4(100.0, 0.0, 0.0, 1.0); // 카메라의 Far에 해당하는 큰 값
        output.pixelColor.r = distance(eyeWorld, input.posWorld);
        return output; // 다른 쉐이딩 생략
    }

    // material.albedo/emission 등을 텍스춰 강도 조절로 사용할 수 있도록 곱해줍니다
    float3 albedo = useAlbedoMap ? albedoTex.SampleLevel(linearSampler, input.texcoord, lod).rgb * material.albedo 
                                 : material.albedo;
    float metallic = useMetallicMap ? metallicRoughnessTex.SampleLevel(linearSampler, input.texcoord, lod).b * material.metallic
                                    : material.metallic;
    float roughness = useRoughnessMap ? metallicRoughnessTex.SampleLevel(linearSampler, input.texcoord, lod).g * material.roughness
                                      : material.roughness;
    float3 emission = useEmissiveMap ? emissiveTex.SampleLevel(linearSampler, input.texcoord, lod).rgb * material.emission
                                     : material.emission;
    float ao = useAOMap ? aoTex.SampleLevel(linearSampler, input.texcoord, lod).r : 1.0;
    
    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao,
                                                  metallic, roughness);

    float3 directLighting = float3(0, 0, 0);

    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        // Directional light
        float3 lightVec = lights[i].type & 0x01 ? -lights[i].direction 
                                              : lights[i].position - input.posWorld;
        float lightDist = length(lightVec);
        lightVec /= lightDist;
        
        float distanceFactor = saturate((lights[i].fallOffEnd - lightDist)
                                     / (lights[i].fallOffEnd - lights[i].fallOffStart));
        
        // Spot light
        float spotFator = lights[i].type & 0x02
                          ? pow(max(-dot(lightVec, lights[i].direction), 0.0f), lights[i].spotPower)
                          : 1.0f;
        
        // Shadow map
        float bias = max(0.05 * (1.0 - dot(normalWorld, lightVec)), 0.02);
        float shadowFactor = lights[i].type & 0x10
                             ? ComputeShadowFactor(input.posWorld, float(i), bias)
                             : 1.0;

        float3 radiance = lights[i].type ? lights[i].radiance * distanceFactor * spotFator * shadowFactor : 0.0;

        directLighting += DirectBRDF(lightVec, pixelToEye, input.posWorld, normalWorld, albedo, metallic, roughness)
                          * radiance;
    }
    
    PixelShaderOutput output;
    output.pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
    
    return output;
}
