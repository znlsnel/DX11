#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

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
   // // 아무것도 하지 않음 (Depth Only)
    return indexColor;
    
}

/* 비교: 반환 자료형 불필요
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
