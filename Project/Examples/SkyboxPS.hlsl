#include "Common.hlsli" // ���̴������� include ��� ����

struct SkyboxPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};
  
cbuffer skyBoxBuffer : register(b4)
{ 
    float3 lightDir;
    float dummy;
}  

PixelShaderOutput main(SkyboxPixelShaderInput input)
{
    PixelShaderOutput output;

     
    if (input.posModel.y < -0.1)
        input.posModel.y *= -1.0f;
    else if (input.posModel.y < 0.1)
        input.posModel.y = 0.1;
     
    output.pixelColor = envIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
output.pixelColor += 0.2;
    /*if (textureToDraw == 0)
        output.pixelColor = envIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    else if (textureToDraw == 1)
        output.pixelColor = specularIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    else if (textureToDraw == 2)
        output.pixelColor = irradianceIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    else
        output.pixelColor = float4(135/255, 206/255, 235/255, 1);*/

    output.pixelColor.rgb *= strengthIBL;
    output.pixelColor.a = 1.0;
    
    return output;
}
