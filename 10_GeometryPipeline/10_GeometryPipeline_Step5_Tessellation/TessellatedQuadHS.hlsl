cbuffer ConstantData : register(b0)
{
    float3 eyeWorld;
    float width;
    Matrix model;
    Matrix view;
    Matrix proj;
    float time = 0.0f;
    float3 padding;
    float4 edges;
    float2 inside;
    float2 padding2;
};

struct VertexOut
{
    float4 pos : POSITION;
};

struct HullOut
{
    float3 pos : POSITION;
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
    
    float len = length(eyeWorld - patch[0].pos.xyz);
    len = len > 2.5 ? 2.5
    : len < 0.0 ? 0.0 : len;
    
    len /= 2.5;
    len = 1.0 - len;
    
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
	
    hout.pos = p[i].pos.xyz;

    return hout;
}
