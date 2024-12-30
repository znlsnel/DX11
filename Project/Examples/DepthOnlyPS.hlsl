#include "Common.hlsli" // 쉐이더에서도 include 사용 가능
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);
Texture2D ARTTex : register(t5);

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
    // 아무것도 하지 않음 (Depth Only)
    float a = useAlbedoMap ? albedoTex.SampleLevel(linearWrapSampler, input.texcoord, 0).a : 1.0;
    float a2 = useARTTexture ? ARTTex.SampleLevel(linearWrapSampler, input.texcoord, 0).r : 1.0;
       
    clip(a - 0.05);
    if (useARTTexture && a2 < 0.1)
        clip(-1);
}

/* 비교: 반환 자료형 불필요
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
