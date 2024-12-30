#include "Ex1606_Common.hlsli"
#include "Common.hlsli" // ���̴������� include ��� ����

#define PI 3.141592

Texture3D<float> signedDistance : register(t5); // t5 ���� ����

cbuffer Consts : register(b3) // b3 ����
{
    float3 uvwOffset; // �̻��
    float lightAbsorptionCoeff = 5.0;
    float3 lightDir = float3(0, 1, 0);
    float densityAbsorption = 10.0;
    float3 lightColor = float3(1, 1, 1) * 40.0;
    float aniso = 0.3;
}

float3 GetUVW(float3 posModel)
{
    return (posModel.xyz + 1.0) * 0.5;
}

float4 ComputeNormal(float3 uvw)
{
    float3 normal;
    normal.x = signedDistance.SampleLevel(linearClampSampler, uvw + float3(dxBase.x * 0.5, 0, 0), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(dxBase.x * 0.5, 0, 0), 0);
    normal.y = signedDistance.SampleLevel(linearClampSampler, uvw + float3(0, dxBase.y * 0.5, 0), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(0, dxBase.y * 0.5, 0), 0);
    normal.z = signedDistance.SampleLevel(linearClampSampler, uvw + float3(0, 0, dxBase.z * 0.5), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(0, 0, dxBase.z) * 0.5, 0);
    return float4(normalize(normal), 0);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 eyeModel = mul(float4(eyeWorld, 1), worldInv).xyz; // ����->�� ����ȯ
    float3 dirModel = normalize(input.posModel - eyeModel);
    
    int numSteps = 256;
    float stepSize = 2.0 / float(numSteps); // �ڽ� ���̰� 2.0
    
    // float absorptionCoeff = 10.0;
    float3 volumeAlbedo = float3(1, 1, 1); //TODO: move to consts
    // float3 lightColor = float3(1, 1, 1) * 40.0;
    
    float4 color = float4(0, 0, 0, 0); // visibility 1.0���� ����
    float3 posModel = input.posModel + dirModel * 1e-6; // ��¦ �� ���¿��� ����

    [loop] // [unroll] ��� �� ���̴� ������ �ʹ� ����
    for (int i = 0; i < numSteps; i++)
    {
        float3 uvw = GetUVW(posModel); // +uvwOffset; �̻�� 

        // ��ü ������
        /* ���� */
        
        float s = signedDistance.SampleLevel(linearClampSampler, uvw, 0).r;

        if (abs(s) < 1e-2) // ǥ�鿡 ����� ���
        {
            float3 normal = ComputeNormal(uvw).xyz;
            
            float3 lightDir = normalize(float3(0, 1, 0) + -dirModel); // ���� ������ ��¦ ���� ������
            
            color = float4(0.2, 0.2, 0.2, 1);
            color.rgb += saturate(dot(normal, lightDir)) * 0.8; // ������ Diffuse ����
            break;
        }
        
        posModel += dirModel * stepSize * clamp(s, -1, 1);
        
        // ���� ������ ������ ���
        if (abs(posModel.x) > 1 || abs(posModel.y) > 1 || abs(posModel.z) > 1)
            break;
    }
    
    return color;
}
