
cbuffer CSData : register(b0)
{
    float velocity;
};

struct Particle
{
    float3 pos;
    float3 color;
};

static float dt = 1 / 300.0; // ConstBuffer로 받아올 수 있음

//StructuredBuffer<Particle> inputParticles : register(t0); // SRV로 사용 가능
RWStructuredBuffer<Particle> outputParticles : register(u0);

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    Particle p = outputParticles[dtID.x]; // Read
    
    // float3 velocity = ...;
    // p.pos += ...;
    float2 RotateDir = float2(-p.pos.y, p.pos.x);
    
    p.pos.xy += RotateDir * velocity * dt;
    
    
    //if (p.pos.x < -1.0)
    //    p.pos.x = 1.0;
    
    outputParticles[dtID.x].pos = p.pos; // Write
}
