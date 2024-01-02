
#include "Common.hlsli"

Texture2D g_heightMap : register(t0);
Texture2DArray ORDp : register(t1);

struct HullShaderOutput
{
    float3 posModel : POSITION1; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
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
float4 main(PatchConstOutput patchConst,
             float2 uv : SV_DomainLocation,
             const OutputPatch<HullShaderOutput, 4> quad) : SV_POSITION0
{
    
    float3 v1 = quad[0].posModel * (1 - uv.x) + quad[1].posModel * (uv.x);
    float3 v2 = quad[2].posModel * (1 - uv.x) + quad[3].posModel * (uv.x);
    float3 posModel = v1 * (1 - uv.y) + v2 * (uv.y);
     
    float2 t1 = quad[0].texcoord * (1 - uv.x) + quad[1].texcoord * (uv.x); 
    float2 t2 = quad[2].texcoord * (1 - uv.x) + quad[3].texcoord * (uv.x);
    float2 texc = t1 * (1 - uv.y) + t2 * (uv.y);
       
     float3 posWorld = mul(float4(posModel, 1.0), world).xyz;
       
    posWorld.y = g_heightMap.SampleLevel(linearWrapSampler, texc / 50, 0).r * 8.0;

    return  mul(float4(posWorld, 1.0), viewProj);
}
