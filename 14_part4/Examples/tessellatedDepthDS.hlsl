
#include "Common.hlsli"

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
    
     float3 posWorld = mul(float4(posModel, 1.0), world).xyz;
    return  mul(float4(posWorld, 1.0), viewProj);
}
