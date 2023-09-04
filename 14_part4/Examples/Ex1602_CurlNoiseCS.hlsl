#include "TileableNoise.hlsli"

RWTexture2D<float4> densityTex : register(u0);

struct Particle
{
    float3 position;
    float3 color;
};

RWStructuredBuffer<Particle> outputParticles : register(u1);

// 가시화를 위해 속도 여러가지로 조절
static float dt = 0.001f; // ConstBuffer로 받아올 수도 있음

float getNoise(float2 uv)
{
    // freq 여러가지로 바꿔보기
    return perlinfbm(float3(uv, 0.0f), 2.0, 1);
}

// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf
float2 getCurl(float2 uv, float2 dx)
{
    // TODO: 완성
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
    
   //densityTex[dtID.xy].xyzw = getNoise(uv) * 1.0f; // 꼬리를 보려면 주석 처리
    
    // 쉐이더 수를 줄이기 위해서 입자들이 width 개라고 가정
    outputParticles[dtID.x].position.xy += getCurl(outputParticles[dtID.x].position.xy, dx) * dt * 0.2;
}

// 쉐이더 토이 참고해서 비슷한 효과 만들어 보기