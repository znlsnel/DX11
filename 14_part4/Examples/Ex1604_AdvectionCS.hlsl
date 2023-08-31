// Advect Velocity and Density

Texture3D<float4> velocityTemp : register(t0);
Texture3D<float> densityTemp : register(t1);

RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

SamplerState pointClampSS : register(s0);
SamplerState linearClampSS : register(s1);
SamplerState linearMirrorSS : register(s2);
SamplerState pointWrapSS : register(s3);
SamplerState linearWrapSS : register(s4);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
    int upScale;
    int numNewParticles;
    float turbulence;
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    velocity.GetDimensions(width, height, depth);
    float3 dx = float3(1.0 / width, 1.0 / height, 1.0 / depth);
    float3 uvw = (dtID.xyz + 0.5) * dx; // �ȼ� �߽�

    // TODO:
    // ����: ���ø��� �ӵ��� dx�� ������� ��
    // float3 vel = ...;
   
    // TODO:
    // �ӵ��� ������ BaseGrid �ػ��̰� Advection�� UpGrid���� �ϱ⶧���� 
    // �ӵ��� upScale�� �����ݴϴ�.
    // float3 uvwBack = ...;
    
    // TODO:
    // density[dtID.xyz] = ...;
    // velocity[dtID.xyz] = ...;
}
