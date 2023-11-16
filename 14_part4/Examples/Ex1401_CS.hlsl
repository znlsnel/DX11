RWTexture2D<float4> gOutput : register(u0);

cbuffer MyBuffer : register(b0)
{
    float scale;
}

// TODO: numthreads 공식 문서 확인

[numthreads(32, 32 , 1)] // TODO: 여러가지로 바꿔보기
void main(int3 gID : SV_GroupID, uint3 tID : SV_DispatchThreadID)
{
    //TODO: groupID, DTid 등을 여러가지로 바꿔가면서 테스트


    if (gID.y % 2 == 0 ^ gID.x % 2 == 0)
    { 
        gOutput[tID.xy] = float4(0.5, 0.5, 0.5, 1.0);
    }
    else
    {
        gOutput[tID.xy] = float4(1, 1, 1, 1) * scale;
    }
}