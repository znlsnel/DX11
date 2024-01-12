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
    
    float len1 = 0.5 * (length(cameraWorld - posWorld1) + length(cameraWorld - posWorld2));
    float len2 = 0.5 * (length(cameraWorld - posWorld2) + length(cameraWorld - posWorld3));
    float len3 = 0.5 * (length(cameraWorld - posWorld3) + length(cameraWorld - posWorld4));
    float len4 = 0.5 * (length(cameraWorld - posWorld4) + length(cameraWorld - posWorld1));
    float len5 = length(cameraWorld - posCenter);
  //  len5 = 1.0; 
     
    float maxSize = 10;
    float distMin = 1.0;
    float distMax = 10.0;
   
    len1 = clamp(len1, 1.0, 10.0);
    len2 = clamp(len2, 1.0, 10.0);
    len3 = clamp(len3, 1.0, 10.0);
    len4 = clamp(len4, 1.0, 10.0);
    len5 = clamp(len5, 1.0, 10.0); 
       
    len1 = 1.0 - saturate((distMax - len1) / (distMax - distMin));
    len2 = 1.0 - saturate((distMax - len2) / (distMax - distMin));
    len3 = 1.0 - saturate((distMax - len3) / (distMax - distMin));
    len4 = 1.0 - saturate((distMax - len4) / (distMax - distMin));
    len5 = 1.0 - saturate((distMax - len5) / (distMax - distMin));
    //len1 /= 3;
    //len2 /= 3;
    //len3 /= 3;
    //len4 /= 3;
    //len5 /= 3; 
        
    pt.edges[0] = lerp(maxSize, 1.0, len2);
    pt.edges[1] = lerp(maxSize, 1.0, len1);
    pt.edges[2] = lerp(maxSize, 1.0, len4);
    pt.edges[3] = lerp(maxSize, 1.0, len3);
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
