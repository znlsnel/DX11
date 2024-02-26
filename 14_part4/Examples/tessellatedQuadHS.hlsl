#include "Common.hlsli"


struct VertexOutput
{
    float3 posModel : POSITION0; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel : NORMAL; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct HullOutput
{
    float3 posModel : POSITION0; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 normalModel : NORMAL; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

 
struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
    //float outside[2] : SV_TessFactor;
};


 
PatchConstOutput MyPatchConstantFunc(InputPatch<VertexOutput, 4> patch,
                                     uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;
    
    float3 posWorld1 = mul(float4(patch[0].posModel, 1.0), world).xyz;
    float3 posWorld2 = mul(float4(patch[1].posModel, 1.0), world).xyz;
    float3 posWorld3 = mul(float4(patch[2].posModel, 1.0), world).xyz;
    float3 posWorld4 = mul(float4(patch[3].posModel, 1.0), world).xyz;
    
    float3 posCenter = (posWorld1 + posWorld2 + posWorld3 + posWorld4) / 4.0;
     
    float len1 = length(cameraWorld - (posWorld1 + (posWorld3 - posWorld1) / 2.0));
    float len2 = length(cameraWorld - (posWorld2 + (posWorld1 - posWorld2) / 2.0));
    float len3 = length(cameraWorld - (posWorld2 + (posWorld4 - posWorld2) / 2.0));
    float len4 = length(cameraWorld - (posWorld3 + (posWorld4 - posWorld3) / 2.0));
    float len5 = length(cameraWorld - posCenter);
      
    float maxSize = 20;
        
    float distMin = 5.0;
    float distMax = 10.0;   
     
    len1 =  saturate((len1 - distMin) / (distMax - distMin));
    len2 =  saturate((len2 - distMin) / (distMax - distMin));
    len3 =  saturate((len3 - distMin) / (distMax - distMin));
    len4 =  saturate((len4 - distMin) / (distMax - distMin));
    len5 =  saturate((len5 - distMin) / (distMax - distMin));
       
    pt.edges[0] = lerp(maxSize, 1.0, len1);
    pt.edges[1] = lerp(maxSize, 1.0, len2);
    pt.edges[2] = lerp(maxSize, 1.0, len3);
    pt.edges[3] = lerp(maxSize, 1.0, len4);
    pt.inside[0] = lerp(maxSize, 1.0, len5);
    pt.inside[1] = lerp(maxSize, 1.0, len5);

    return pt;
}

[domain("quad")] 
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)]
HullOutput main(InputPatch<VertexOutput, 4> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HullOutput output;
    output.normalModel = p[i].normalModel;
    output.posModel = p[i].posModel;
    output.tangentModel = p[i].tangentModel;
    output.texcoord = p[i].texcoord;
    return output;

}
