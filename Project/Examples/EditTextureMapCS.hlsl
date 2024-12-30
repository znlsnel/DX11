RWTexture2D<float4> textureMap : register(u0);

cbuffer ComputeShaderInput : register(b0)
{
    float2 pos;
    float radius;
    float type;
}


[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, uint3 tID : SV_DispatchThreadID)
{
    float distance = length(float2(tID.xy) - pos);
    
    if (distance <= radius)
    {
        uint2 pos = uint2(tID.x, tID.y);
        textureMap[pos] = float4(type, type, type, type);
    }
}