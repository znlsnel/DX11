#include "Common.hlsli" // ���̴������� include ��� ����

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_POSITION;
};

//struct PixelShaderOutput
//{
//    float4 IndexColor : SV_Target0;
//};

void main(DepthOnlyPixelShaderInput input) 
{
   // // �ƹ��͵� ���� ���� (Depth Only)
   // PixelShaderOutput output;

   //// output.IndexColor = indexColor;
   // output.IndexColor = float4(0.1, 0.2, 0.3, 0.4);
    
   // return output;
    
}

/* ��: ��ȯ �ڷ��� ���ʿ�
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
