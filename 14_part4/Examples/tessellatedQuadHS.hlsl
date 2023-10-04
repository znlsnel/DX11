
const float4 edges = { 64.0, 64.0, 64.0, 64.0 };
const float2 inside = { 64.0, 64.0 };

struct VertexOut
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; // Volume casting 시작점
};

struct HullOut
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; // Volume casting 시작점
};

struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};



PatchConstOutput MyPatchConstantFunc(InputPatch<VertexOut, 4> patch,
                                     uint patchID : SV_PrimitiveID)
{
    PatchConstOutput pt;
    
    //float len = length(patch[patchID] - patch[0].posWorld);
    
    //len = clamp(len, 0.0, 2.5);
    //len /= 2.5;
    //len = 1.0 - len;
    
    float len = 0.0;
    
    pt.edges[0] = lerp(1.0, edges[0], len);
    pt.edges[1] = lerp(1.0, edges[1], len);
    pt.edges[2] = lerp(1.0, edges[2], len);
    pt.edges[3] = lerp(1.0, edges[3], len);
    pt.inside[0] = lerp(1.0, inside[0], len);
    pt.inside[1] = lerp(1.0, inside[1], len);
	
    return pt;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("MyPatchConstantFunc")]
[maxtessfactor(64.0f)]
HullOut main(InputPatch<VertexOut, 4> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HullOut hout;
	
    hout.normalWorld = p[i].normalWorld;
    hout.posModel = p[i].posModel;
    hout.posProj = p[i].posProj;
    hout.posWorld = p[i].posWorld;
    hout.tangentWorld = p[i].tangentWorld;
    hout.texcoord = p[i].texcoord;

    return hout;
}
