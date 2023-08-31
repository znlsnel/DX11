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
    float3 color; // �̻��
    float life;
    float size;
};

StructuredBuffer<Particle> particles : register(t0);

// ����: VertexBuffer�� Dynamic���� ����� �� �ֽ��ϴ�.

// VSInput�� ���� vertexID�� ���
PSInput main(uint vertexID : SV_VertexID)
{
    const float fadeLife = 0.5f;
    
    Particle p = particles[vertexID];
    
    PSInput output;
    
    output.position = float4(p.position.xyz, 1.0);
    
    float3 color1 = float3(1.0f, 0.5f, 0.2);
    float3 color2 = float3(1.0f, 0.3f, 0);
    
    float3 tempColor = lerp(color2, color1, pow(saturate(p.life / 1.0f), 2.0));
    
    output.color = tempColor * saturate(p.life / fadeLife);

    output.life = p.life;
    output.size = p.size;

    return output;
}
