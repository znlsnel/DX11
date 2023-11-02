#include "Common.hlsli"

Texture2D g_heightMapTexture : register(t0);
Texture2D g_meshHeightTexture : register(t1);


VertexShaderInput main(VertexShaderInput vin)
{
    return vin;
}

PixelShaderInput mainS(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posModel = input.posModel;
    
    output.posWorld = mul(float4(output.posModel, 1.0), world).xyz;
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
     
    // TangentWorld
    { 

        output.tangentWorld = mul(float4(input.tangentModel, 1.0), world).xyz;

    }
    
    // NormalWorld
    {
        output.normalWorld = mul(float4(input.normalModel, 1.0), worldIT);
        output.normalWorld = normalize(output.normalWorld);
    }
    
        // texcoord
    {

        output.texcoord = input.texcoord;
    }
    
    if (useHeightMap)
    {
        float height = g_meshHeightTexture.SampleLevel(linearClampSampler, output.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        output.posWorld += output.normalWorld * height * heightScale;
    }
    
    return output;
}
