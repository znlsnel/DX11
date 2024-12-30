#include "TileableNoise.hlsli"

RWTexture2D<float4> densityTex : register(u0);

struct Particle
{
    float3 position;
    float3 color;
};

RWStructuredBuffer<Particle> outputParticles : register(u1);

// ����ȭ�� ���� �ӵ� ���������� ����
static float dt = 0.001f; // ConstBuffer�� �޾ƿ� ���� ����

float getNoise(float2 uv)
{
    // freq ���������� �ٲ㺸��
    return perlinfbm(float3(uv, 0.0f), 2.0, 1);
}

// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf
float2 getCurl(float2 uv, float2 dx)
{
    // TODO: �ϼ�
    float left = getNoise(float2(uv.x - dx.x, uv.y));
    float right = getNoise(float2(uv.x + dx.x, uv.y));
    float up = getNoise(float2(uv.x, uv.y + dx.y * 0.5));
    float down = getNoise(float2(uv.x, uv.y - dx.y * 0.5));

    float2 result = float2((right - left) / dx.x * 0.5, (up - down) / dx.y * 0.5);
    
    return float2(result.y, -result.x);
}

[numthreads(32, 32, 1)]
void main(uint3 dtID : SV_DispatchThreadID, uint gIndex : SV_GroupIndex)
{
    uint width, height;
    densityTex.GetDimensions(width, height);
    
    float2 uv = dtID.xy / float2(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);
    
   //densityTex[dtID.xy].xyzw = getNoise(uv) * 1.0f; // ������ ������ �ּ� ó��
    
    // ���̴� ���� ���̱� ���ؼ� ���ڵ��� width ����� ����
    outputParticles[dtID.x].position.xy += getCurl(outputParticles[dtID.x].position.xy, dx) * dt * 0.2;
}

// ���̴� ���� �����ؼ� ����� ȿ�� ����� ����