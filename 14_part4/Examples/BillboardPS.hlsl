#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

cbuffer BillboardContsts : register(b3)
{
    float widthWorld;
    float3 dirWorld;
    int index;
    float3 dummy;
};
struct BillboardPixelShaderInput
{
    float4 pos : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float4 center : POSITION1;
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
    float4  indexColor : SV_Target1;
};
 
PixelShaderOutput main(BillboardPixelShaderInput input)
{
    float3 uvw = float3(input.texCoord, float(index % 5));
    float4 pixelColor = g_treeTexArray.Sample(linearWrapSampler, uvw);
    // clip(x)에서 x가 0보다 작으면 이 픽셀의 색은 버린다.     
    
    // alpha 값이 있는 이미지에서 불투명도가 0.9보다 작으면 clip
    clip(pixelColor.a - 0.9f);
    
    // 픽셀의 값이 흰색에 가까운 배경 색이면 clip
    //TODO: clip(...)
    float i = (pixelColor.r + pixelColor.g + pixelColor.b) / 3.0;
    clip((i > 0.9) ? -1 : 1);
    
    //clip((pixelColor.a < 0.9f) || (pixelColor.r + pixelColor.g + pixelColor.b) > 2.4 ? -1 : 1);
    
    PixelShaderOutput output;
    
    output.pixelColor = pixelColor;
    //output.indexColor = float4(9.0/255.0, 0.0, 0.0, 1.0);
    output.indexColor = indexColor;
    //output.indexColor = float4(1.0, 0.9, 0.1, 1.0);

    return output;
} 
 