#include "Common.hlsli" // ���̴������� include ��� ����
Texture2D albedoTex : register(t0);

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

//struct PixelShaderOutput
//{
//    float4 IndexColor : SV_Target0;
//};

void main(DepthOnlyPixelShaderInput input)
{
   // // �ƹ��͵� ���� ���� (Depth Only)
    float a = albedoTex.SampleLevel(linearWrapSampler, input.texcoord, 0).a;

    clip(a - 0.05);

}

/* ��: ��ȯ �ڷ��� ���ʿ�
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
