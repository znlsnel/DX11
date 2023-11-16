Texture2D<float2> velocity : register(t0);
RWTexture2D<float> divergence : register(u0);
RWTexture2D<float> pressure : register(u1);
RWTexture2D<float> pressureTemp : register(u2);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    divergence.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);

    uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
    uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
    uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
    uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);

    float2 Dleft = velocity.SampleLevel(linearWrapSS, left, 0);
    float2 DRight = velocity.SampleLevel(linearWrapSS, right, 0);
    float2 DUp = velocity.SampleLevel(linearWrapSS, up, 0); 
    float2 DDown = velocity.SampleLevel(linearWrapSS, down, 0);

    
     
    //divergence[dtID.xy] = 0.5 * (DRight - Dleft).x
    //+ (DUp - DDown).y;
    
    divergence[dtID.xy] = 0.5 * (velocity[right].x - velocity[left].x + velocity[up].y - velocity[down].y);
     
    pressure[dtID.xy] = 0.0;
    pressureTemp[dtID.xy] = 0.0;
}
