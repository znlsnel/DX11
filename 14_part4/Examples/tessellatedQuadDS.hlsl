
#include "Common.hlsli"


Texture2D g_heightMap : register(t0);
Texture2DArray ORDp : register(t1);
  
struct HullShaderOutput
{
    float3 posModel : POSITION; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel : NORMAL; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    //float inside[2] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
    
};

[domain("quad")]
PixelShaderInput main(PatchConstOutput patchConst,
             float2 uv : SV_DomainLocation,
             const OutputPatch<HullShaderOutput, 4> quad)
{ 
    PixelShaderInput output;
    
    float3 v1 = quad[0].posModel * (1 - uv.x) + quad[1].posModel * (uv.x);
    float3 v2 = quad[2].posModel * (1 - uv.x) + quad[3].posModel * (uv.x);
    output.posModel = v1 * (1 - uv.y) + v2 * (uv.y);
    
    output.posWorld = mul(float4(output.posModel, 1.0), world).xyz;
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
     
    // TangentWorld
    { 
        float4 temp1 = mul(float4(quad[0].tangentModel, 0.0), world);
        float4 temp2 = mul(float4(quad[1].tangentModel, 0.0), world);
        float4 temp3 = mul(float4(quad[2].tangentModel, 0.0), world);
        float4 temp4 = mul(float4(quad[3].tangentModel, 0.0), world);
        
        
        temp1 = temp1 * (1 - uv.x) + temp2 * (uv.x);
        temp3 = temp3 * (1 - uv.x) + temp4 * (uv.x);
        output.tangentWorld = (temp1 * (1 - uv.y) + temp3 * (uv.y)).xyz;
    }
    
    // NormalWorld
    {
        float4 temp1 = mul(float4(quad[0].normalModel, 0.0), worldIT);
        float4 temp2 = mul(float4(quad[1].normalModel, 0.0), worldIT);
        float4 temp3 = mul(float4(quad[2].normalModel, 0.0), worldIT);
        float4 temp4 = mul(float4(quad[3].normalModel, 0.0), worldIT);
    
        temp1 = temp1 * (1 - uv.x) + temp2 * (uv.x);
        temp3 = temp3 * (1 - uv.x) + temp4 * (uv.x);
        output.normalWorld = (temp1 * (1 - uv.y) + temp3 * (uv.y)).xyz;
        output.normalWorld = normalize(output.normalWorld);
    }
    
        // texcoord
    {
        float2 temp1 = quad[0].texcoord * (1 - uv.x) + quad[1].texcoord * (uv.x);
        float2 temp2 = quad[2].texcoord * (1 - uv.x) + quad[3].texcoord * (uv.x);
        output.texcoord = (temp1 * (1 - uv.y) + temp2 * (uv.y));
    }
       
    //if (useHeightMap) 
    //{    
    //    float temp = g_heightMap.SampleLevel(linearWrapSampler, output.texcoord / 100, 0).r;
    //    //int currTexture = round(temp * 255);
    //    float height = ORDp.SampleLevel(linearWrapSampler, float3(output.texcoord, currTexture), 0).b;
    //    height = height * 2.0 - 1.0;
    //    output.posWorld += output.normalWorld * height * heightScale; 
    //    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    //} 
        
    float height = g_heightMap.SampleLevel(linearWrapSampler, output.texcoord / 50, 0).r * 8.0;
    output.posWorld.y = height;
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    
    return output;
     
} 
 