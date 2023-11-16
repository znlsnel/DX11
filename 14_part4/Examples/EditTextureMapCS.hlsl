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
    //// ComputeShader에서의 Thread ID를 계산합니다.
    //uint2 threadID = dtID.xy;

    //// textureMap의 크기
    //uint2 textureSize = uint2(0, 0);
    //textureMap.GetDimensions(textureSize.x, textureSize.y);

    //// 현재 Thread가 처리해야 할 Pixel의 좌표를 계산합니다.
    //uint2 pixelPos = threadID + gIndex * 32;

    //// 계산된 Pixel의 좌표가 textureMap의 범위를 벗어나면 종료합니다.
    //// Pixel의 좌표와 cbuffer로 받아온 pos의 거리를 계산합니다.
     
    float distance = length(float2(tID.xy) - pos);
    
    if (distance <= radius)
    {
        uint2 pos = uint2(tID.x, tID.y);
        textureMap[pos] = float4(type, type, type, type);
    }

    
}