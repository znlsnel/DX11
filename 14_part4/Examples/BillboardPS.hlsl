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

    clip(pixelColor.a - 0.9f);
    
    float i = (pixelColor.r + pixelColor.g + pixelColor.b) / 3.0;
    clip((i > 0.9) ? -1 : 1);
    
    PixelShaderOutput output;
    output.pixelColor = pixelColor;
    output.indexColor = indexColor;

    return output;
}  
 
// 