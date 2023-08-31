// Advect Velocity and Density

Texture2D<float2> velocityTemp : register(t0);
Texture2D<float4> densityTemp : register(t1);

RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);

// Repeated Boundary
SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

cbuffer Consts : register(b0)
{
    float dt;
    float viscosity;
    float alpha;
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    velocity.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);
    float2 pos = (dtID.xy + 0.5) * dx; // �� �����尡 ó���ϴ� ���� �߽�
    
    // TODO: 1. velocityTemp�κ��� �ӵ� ���ø��ؿ���
    float2 vel = float2(1.0, 0);
    
    // TODO: 2. �� �ӵ��� �̿��ؼ� ������ ��ġ ���
    // float2 posBack = ...;

    // TODO: 3. �� ��ġ���� ���ø� �ؿ���
    // velocity[dtID.xy] = ...;
    // density[dtID.xy] = ...;
}
