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
    float3 center = (patch[0].pos + patch[1].pos + patch[2].pos + patch[3].pos).xyz * 0.25;
    float dist = length(center - eyeWorld);
    float distMin = 0.5;
    float distMax = 2.0;
    float tess = 64.0 * saturate((distMax - dist) / (distMax - distMin)) + 1.0;
    
    PatchConstOutput pt;
	
    pt.edges[0] = tess;
    pt.edges[1] = tess;
    pt.edges[2] = tess;
    pt.edges[3] = tess;
	
    pt.inside[0] = tess;
    pt.inside[1] = tess;
    
    /*pt.edges[0] = edges[0];
    pt.edges[1] = edges[1];
    pt.edges[2] = edges[2];
    pt.edges[3] = edges[3];	
    pt.inside[0] = inside[0];
    pt.inside[1] = inside[1];*/
	
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
