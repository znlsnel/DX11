Texture2D<float2> velocityTemp : register(t0);
Texture2D<float4> densityTemp : register(t1);
RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

cbuffer Consts : register(b0)
{
    float dt;
    float viscosity;
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height; 
    velocity.GetDimensions(width, height);
    
    uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
    uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
    uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
    uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
    
    // Explicit integration
    velocity[dtID.xy] = (velocityTemp[dtID.xy] + viscosity * dt * (velocityTemp[left]
                 + velocityTemp[right] + velocityTemp[up]
                + velocityTemp[down])) / (1.0 + 4 * viscosity * dt);
    
    // ���ǻ� Density�� �Բ� ����
    // Density�� diffusion���� viscosity�� ������ ���(coefficient) ��� ����
    density[dtID.xy] = (densityTemp[dtID.xy] + viscosity * dt * (densityTemp[left] 
                 + densityTemp[right] + densityTemp[up] 
                + densityTemp[down])) / (1.0 + 4 * viscosity * dt);
    
    
}
