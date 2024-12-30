struct PSInput // GS�� �ִٸ� GSInput���� ����
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float life : PSIZE0;
    float size : PSIZE1;
};

struct Particle
{
    float3 position;
    float3 velocity;
    float3 color;
    float life;
    float size;
};

StructuredBuffer<Particle> particles : register(t0);

// VSInput�� ���� vertexID�� ���
PSInput main(uint vertexID : SV_VertexID)
{
    const float fadeLife = 0.2f;
    
    Particle p = particles[vertexID];
    
    PSInput output;
    
    output.position = float4(p.position.xyz, 1.0);
    output.color = p.color * saturate(p.life / fadeLife);
    output.life = p.life;
    output.size = p.size;

    return output;
}
