#include "Common.hlsli"

Texture2D g_heightMapTexture : register(t0);

struct VertexOutput
{
    float3 posModel : POSITION0; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 posModelHeightMap : POSITION1;
    float3 normalModel : NORMAL; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

VertexOutput main(VertexShaderInput vin)
{
    VertexOutput output;
    output.normalModel = vin.normalModel;
    output.posModel = vin.posModel;
    output.posModelHeightMap = vin.posModel;
    output.tangentModel = vin.tangentModel;
    output.texcoord = vin.texcoord;

     
    //float2 temp = vin.texcoord / 30.0;
    //float height = g_heightMapTexture.SampleLevel(linearWrapSampler, temp, 0).r;
    //height = height * 2.0 - 1.0;
    //output.posModelHeightMap = vin.posModel + vin.normalModel * height * 5;

    return output;
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
    
    //if (useHeightMap)
    //{
    //    float height = g_meshHeightTexture.SampleLevel(linearClampSampler, output.texcoord, 0).r;
    //    height = height * 2.0 - 1.0;
    //    output.posWorld += output.normalWorld * height * heightScale;
    //}
    
    return output;
}
