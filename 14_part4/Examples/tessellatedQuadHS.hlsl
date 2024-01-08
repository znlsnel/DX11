#include "Common.hlsli"


struct VertexOutput
{
    float3 posModel : POSITION0; //�� ��ǥ���� ��ġ position
    float3 normalModel : NORMAL; // �� ��ǥ���� normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct HullOutput
{
    float3 posModel : POSITION0; //�� ��ǥ���� ��ġ position
    float3 normalModel : NORMAL; // �� ��ǥ���� normal    
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
    
    float len1 = 0.5 * (length(eyeWorld - posWorld1) + length(eyeWorld - posWorld2));
    float len2 = 0.5 * (length(eyeWorld - posWorld2) + length(eyeWorld - posWorld3));
    float len3 = 0.5 * (length(eyeWorld - posWorld3) + length(eyeWorld - posWorld4));
    float len4 = 0.5 * (length(eyeWorld - posWorld4) + length(eyeWorld - posWorld1));
    float len5 = length(eyeWorld - posCenter);
  //  len5 = 1.0; 
    float distMin = 1.0;
    float distMax = 10.0;
    
    
    len1 = 1.0 -  saturate((distMax - len1) / (distMax - distMin));
    len2= 1.0 - saturate((distMax - len2) / (distMax - distMin));
    len3 = 1.0 - saturate((distMax - len3) / (distMax - distMin));
    len4 = 1.0 - saturate((distMax - len4) / (distMax - distMin));
    len5 = 1.0 - saturate((distMax - len5) / (distMax - distMin));
    len1 /= 3; 
    len2 /= 3;
    len3 /= 3;
    len4 /= 3;
    len5 /= 3;
      
    float maxSize = 20;
    //pt.edges[0] = lerp(1.0, maxSize, len2);
    //pt.edges[1] = lerp(1.0, maxSize, len1);
    //pt.edges[2] = lerp(1.0, maxSize, len4);
    //pt.edges[3] = lerp(1.0, maxSize, len3);
    //pt.inside[0] = lerp(1.0, maxSize, len5);
    //pt.inside[1] = lerp(1.0, maxSize, len5);
    pt.edges[0] = lerp(maxSize,1.0, len2);
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
