#include "Common.hlsli" // ���̴������� include ��� ����

TextureCube g_diffuseCube : register(t0);
TextureCube g_specularCube : register(t1);

Texture2D g_albedoTexture : register(t2);
Texture2D g_normalTexture : register(t3);
Texture2D g_aoTexture : register(t4);

SamplerState g_sampler : register(s0);

cbuffer BasicPixelConstantData : register(b0)
{
    float3 eyeWorld;
    float mipmapLevel;
    Material material;
    Light light[MAX_LIGHTS];
    float4 indexColor; // ��ŷ(Picking)�� ���
    int useAlbedoTexture;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int dummy;
};

// Schlick approximation: Eq. 9.17 in "Real-Time Rendering 4th Ed."
// fresnelR0�� ������ ���� ����
// Water : (0.02, 0.02, 0.02)
// Glass : (0.08, 0.08, 0.08)
// Plastic : (0.05, 0.05, 0.05)
// Gold: (1.0, 0.71, 0.29)
// Silver: (0.95, 0.93, 0.88)
// Copper: (0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 fresnelR0, float3 normal, float3 toEye)
{
    // ���� �ڷ��
    // THE SCHLICK FRESNEL APPROXIMATION by Zander Majercik, NVIDIA
    // http://psgraphics.blogspot.com/2020/03/fresnel-equations-schlick-approximation.html

    float normalDotView = saturate(dot(normal, toEye));

    float f0 = 1.0f - normalDotView; // 90���̸� f0 = 1, 0���̸� f0 = 0

    // 1.0 ���� ���� ���� ���� �� ���ϸ� �� ���� ���� �˴ϴ�.
    // 0�� -> f0 = 0 -> fresnelR0 ��ȯ
    // 90�� -> f0 = 1.0 -> float3(1.0) ��ȯ
    // 0���� ����� �����ڸ��� Specular ����, 90���� ����� ������ ����
    // ����(fresnelR0)
    return fresnelR0 + (1.0f - fresnelR0) * pow(f0, 5.0);
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
    float4 indexColor : SV_Target1;
};


PixelShaderOutput main(PixelShaderInput input)
{
    float3 toEye = normalize(eyeWorld - input.posWorld);

    float3 color = float3(0.0, 0.0, 0.0);

    int i = 0;

    float3 normalWorld = input.normalWorld;
    
    if (useNormalMap) // NormalWorld�� ��ü
    {
        float3 normalTex = g_normalTexture.SampleLevel(g_sampler, input.texcoord, 0.0).rgb;
         normalTex = 2.0 * normalTex - 1.0; // ���� ���� [-1.0, 1.0]
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normalTex, TBN));
    }
    
    [unroll] 
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(light[i], material, normalWorld,
                                         toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  ++i)
    {
        color += ComputePointLight(light[i], material, input.posWorld,
                                   normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS;
                  ++i)
    {
        color += ComputeSpotLight(light[i], material, input.posWorld,
                                  normalWorld, toEye);
    }

    // ���� ������ �� �ִ� ������ �����Դϴ�.
    // IBL�� �ٸ� ���̵� ���(��: �� ���̵�)�� ���� ����� ���� �ֽ��ϴ�.

    float4 diffuse = g_diffuseCube.Sample(g_sampler, normalWorld) + float4(color, 1.0);
    float4 specular =
        g_specularCube.Sample(g_sampler, reflect(-toEye, normalWorld));

    diffuse *= float4(material.diffuse, 1.0);
    specular *=
        pow(abs(specular.r + specular.g + specular.b) / 3.0, material.shininess);
    specular *= float4(material.specular, 1.0);

    // ����: https://www.shadertoy.com/view/lscBW4
    float3 f = SchlickFresnel(material.fresnelR0, normalWorld, toEye);
    specular.xyz *= f;

    float dist = length(eyeWorld - input.posWorld);
    float distMin = 5.0;
    float distMax = 20.0;
    float lod = 10.0 * saturate(dist / (distMax - distMin));
    
    if (useAlbedoTexture)
    {
        //diffuse *= g_albedoTexture.SampleLevel(g_sampler, input.texcoord, lod);
        // �̹� ���������� ���� ���� �ػ� ���
        diffuse *= g_albedoTexture.SampleLevel(g_sampler, input.texcoord, 0.0);
    }
    
    // ���� https://github.com/microsoft/DirectXTK/blob/main/Src/Shaders/PBRCommon.fxh#L132
    if (useAOMap)
        diffuse *= g_aoTexture.SampleLevel(g_sampler, input.texcoord, lod);

    PixelShaderOutput output;
    output.pixelColor = diffuse + specular;
    output.indexColor = indexColor;
    
    return output;
}
