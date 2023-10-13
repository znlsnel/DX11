#include "Common.hlsli" // ���̴������� include ��� ����

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_POSITION;
};

struct PixelShaderOutput
{
    float4 IndexColor : SV_Target0;
};

float4 main(DepthOnlyPixelShaderInput input) : SV_Target0
{
   // // �ƹ��͵� ���� ���� (Depth Only)
    return indexColor;
    
}

/* ��: ��ȯ �ڷ��� ���ʿ�
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
