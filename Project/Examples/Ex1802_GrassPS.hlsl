#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

struct GrassPixelInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION;
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
};

float4 main(GrassPixelInput input) : SV_TARGET
{
    float3 lightDir = normalize(float3(0.2, 1.0, 0.0));
    
    float3 baseColor = float3(0, 1, 0) * pow(saturate(input.texcoord.y), 3.0);
    
    // 간단한 directional light, 양면이라서 abs 사용
    float3 color = baseColor * abs(dot(input.normalWorld, lightDir)) * 2;
    return float4(color, 1); 
}
 
/*
float4 main(GrassPixelInput input) : SV_TARGET
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    
    float3 normalWorld = dot(pixelToEye, input.normalWorld) > 0 ? input.normalWorld : -input.normalWorld; // 양면
    
    //float3 lightDir = normalize(float3(0.2, 1.0, 0.0));
    float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;
    
    // 간단한 directional light, 양면이라서 abs 사용
    float3 color = input.baseColor * irradiance * dot(pixelToEye, normalWorld) * strengthIBL;
    return float4(color, 1);
}
*/