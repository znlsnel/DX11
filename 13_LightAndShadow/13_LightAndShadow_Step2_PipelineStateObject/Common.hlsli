#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// ���̴����� include�� ������� .hlsli ���Ͽ� �ۼ�
// Properties -> Item Type: Does not participate in build���� ����

#define MAX_LIGHTS 3 // ���̴������� #define ��� ����
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

// ���÷����� ��� ���̴����� �������� ���
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);

// ���� �ؽ���� t10 ���� ����
TextureCube envIBLTex : register(t10);
TextureCube specularIBLTex : register(t11);
TextureCube irradianceIBLTex : register(t12);
Texture2D brdfTex : register(t13);

struct Light
{
    float3 radiance; // Strength
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

// ���� Constants
cbuffer GlobalConstants : register(b1)
{
    matrix view;
    matrix proj;
    matrix viewProj;
    float3 eyeWorld;
    float strengthIBL;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, �׿�: ������
    float envLodBias = 0.0f; // ȯ��� LodBias
    float lodBias = 2.0f; // �ٸ� ��ü�� LodBias
    float dummy2;
    
    Light light[MAX_LIGHTS];
};

struct VertexShaderInput
{
    float3 posModel : POSITION; //�� ��ǥ���� ��ġ position
    float3 normalModel : NORMAL0; // �� ��ǥ���� normal    
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION; // World position (���� ��꿡 ���)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
};

#endif // __COMMON_HLSLI__