#include "Common.hlsli"

struct VertexOutput
{
    float3 posModel : POSITION0; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel : NORMAL; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

VertexOutput main(VertexShaderInput vin)
{
    VertexOutput output;
    output.normalModel = vin.normalModel;
    output.posModel = vin.posModel;
    output.tangentModel = vin.tangentModel;
    output.texcoord = vin.texcoord;

     
    //float2 temp = vin.texcoord / 30.0;
    //float height = g_heightMapTexture.SampleLevel(linearWrapSampler, temp, 0).r;
    //height = height * 2.0 - 1.0;
    //output.posModelHeightMap = vin.posModel + vin.normalModel * height * 5;

    return output;
}
