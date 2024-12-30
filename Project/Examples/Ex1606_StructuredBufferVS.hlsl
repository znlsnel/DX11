#include "Common.hlsli"
#include "Ex1606_Common.hlsli"

struct PSInput // GS�� �ִٸ� GSInput���� ����
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION0;
};

StructuredBuffer<Particle> particles : register(t0);
StructuredBuffer<SortElement> sortElements : register(t1);

// VSInput�� ���� vertexID�� ���
PSInput main(uint vertexID : SV_VertexID)
{
    SortElement e = sortElements[vertexID];
    
    Particle p = particles[e.value];
    
    // ���� ��ǥ��� [0,width]x[...]x[...]  �� object space�� ����
    p.pos.xyz *= dxBase * 2.0;
    p.pos.xyz -= 1.0;
    
    PSInput output;
    
    output.posModel = p.pos.xyz;
    output.posProj = mul(mul(float4(p.pos.xyz, 1.0), world), viewProj);

    return output;
}
