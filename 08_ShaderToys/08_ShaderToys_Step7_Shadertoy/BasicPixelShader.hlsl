#include "Common.hlsli" // ���̴������� include ��� ����

Texture2D g_texture0 : register(t0);
TextureCube g_diffuseCube : register(t1);
TextureCube g_specularCube : register(t2);
SamplerState g_sampler : register(s0);

cbuffer BasicPixelConstantData : register(b0)
{
    float3 eyeWorld;
    bool useTexture;
    Material material;
    Light light[MAX_LIGHTS];
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

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 toEye = normalize(eyeWorld - input.posWorld);

    float3 color = float3(0.0, 0.0, 0.0);

    int i = 0;

    [unroll] 
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(light[i], material, input.normalWorld,
                                         toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  ++i)
    {
        color += ComputePointLight(light[i], material, input.posWorld,
                                   input.normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS;
                  ++i)
    {
        color += ComputeSpotLight(light[i], material, input.posWorld,
                                  input.normalWorld, toEye);
    }

    // ���� ������ �� �ִ� ������ �����Դϴ�.
    // IBL�� �ٸ� ���̵� ���(��: �� ���̵�)�� ���� ����� ���� �ֽ��ϴ�.

    float4 diffuse = g_diffuseCube.Sample(g_sampler, input.normalWorld);
    float4 specular =
        g_specularCube.Sample(g_sampler, reflect(-toEye, input.normalWorld));

    diffuse *= float4(material.diffuse, 1.0);
    specular *=
        pow(abs(specular.r + specular.g + specular.b) / 3.0, material.shininess);
    specular *= float4(material.specular, 1.0);

    // ����: https://www.shadertoy.com/view/lscBW4
    float3 f = SchlickFresnel(material.fresnelR0, input.normalWorld, toEye);
    specular.xyz *= f;

    if (useTexture)
    {
        diffuse *= g_texture0.Sample(g_sampler, input.texcoord);
        // Specular texture�� ������ ����� ���� �ֽ��ϴ�.
    }

    return diffuse + specular;

    // return useTexture ? float4(color, 1.0) * g_texture0.Sample(g_sampler,
    // input.texcoord) : float4(color, 1.0);
}
