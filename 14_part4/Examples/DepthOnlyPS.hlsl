#include "Common.hlsli" // ���̴������� include ��� ����

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_POSITION;
};

void main(float4 pos : SV_POSITION)
{
    // �ƹ��͵� ���� ���� (Depth Only)
}

/* ��: ��ȯ �ڷ��� ���ʿ�
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
