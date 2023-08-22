#include "Common.hlsli" 

// t20�������� ����
Texture2D renderTex : register(t20); // Rendering results
Texture2D depthOnlyTex : register(t21); // DepthOnly

cbuffer PostEffectsConstants : register(b3)
{
    int mode; // 1: Rendered image, 2: DepthOnly
    float depthScale;
    float fogStrength;
};

struct SamplingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 TexcoordToView(float2 texcoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1; // ����: y ������ ��������� �մϴ�.
    posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView;
}

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    if (mode == 1)
    {
        float4 posView = TexcoordToView(input.texcoord);
        float dist = length(posView.xyz); // ���� ��ġ�� ������ ��ǥ��

     
        // TODO: Fog
        return renderTex.Sample(linearClampSampler, input.texcoord);
    }
    else // if (mode == 2)
    {
        float z = TexcoordToView(input.texcoord).z * depthScale;
        return float4(z, z, z, 1);
    }
}