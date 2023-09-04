RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);

cbuffer Consts : register(b0)
{
    float dt;
    float viscosity;
    float2 sourcingVelocity;
    float4 sourcingDensity;
    uint i;
    uint j;
}

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(32, 32, 1)] 
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    density.GetDimensions(width, height);

    // �ణ�� Dissipation
   // density[dtID.xy] = max(0.0, density[dtID.xy] - 0.001); 
    
    // unsigned int�� ���콺 �Է��� ���� ��� CPU �ڵ忡�� i = -1
    // �����÷ο�� ���� width ���� ū ������ ����
    int radius = 50;
    if (i < width)
    {
        float dist = length(float2(dtID.xy) - float2(i, j)) / radius;
        float scale = smootherstep(1.0 - dist);

        velocity[dtID.xy] += sourcingVelocity * scale;
        density[dtID.xy] += sourcingDensity * scale;
    }
}
