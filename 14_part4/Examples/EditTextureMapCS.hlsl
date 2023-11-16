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
    //// ComputeShader������ Thread ID�� ����մϴ�.
    //uint2 threadID = dtID.xy;

    //// textureMap�� ũ��
    //uint2 textureSize = uint2(0, 0);
    //textureMap.GetDimensions(textureSize.x, textureSize.y);

    //// ���� Thread�� ó���ؾ� �� Pixel�� ��ǥ�� ����մϴ�.
    //uint2 pixelPos = threadID + gIndex * 32;

    //// ���� Pixel�� ��ǥ�� textureMap�� ������ ����� �����մϴ�.
    //// Pixel�� ��ǥ�� cbuffer�� �޾ƿ� pos�� �Ÿ��� ����մϴ�.
     
    float distance = length(float2(tID.xy) - pos);
    
    if (distance <= radius)
    {
        uint2 pos = uint2(tID.x, tID.y);
        textureMap[pos] = float4(type, type, type, type);
    }

    
}