#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

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
   // // 아무것도 하지 않음 (Depth Only)
   // PixelShaderOutput output;

   //// output.IndexColor = indexColor;
   // output.IndexColor = float4(0.1, 0.2, 0.3, 0.4);
    
   // return output;
    
}

/* 비교: 반환 자료형 불필요
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
