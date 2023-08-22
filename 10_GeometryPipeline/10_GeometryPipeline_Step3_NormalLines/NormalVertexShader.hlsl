#include "Common.hlsli"

cbuffer BasicVertexConstantData : register(b0)
{
    matrix modelWorld;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

cbuffer NormalVertexConstantData : register(b1)
{
    float scale; // 그려지는 선분의 길이 조절
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION; //모델 좌표계의 위치 position
    float3 normalWorld : NORMAL; // 모델 좌표계의 normal    
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    output.posModel = float4(input.posModel, 1.0);
    output.normalWorld = input.normalModel;
    return output;
    //float4 pos = float4(input.posModel, 1.0f);

    //// Normal 먼저 변환
    //float4 normal = float4(input.normalModel, 0.0f);
    //output.normalWorld = mul(normal, invTranspose).xyz;
    ////output.normalWorld = mul(normal, modelWorld).xyz;
    //output.normalWorld = normalize(output.normalWorld);
        
    //pos = mul(pos, modelWorld);
    
    //float t = input.texcoord.x;
    
    //pos.xyz += output.normalWorld * t * scale;

    //output.posWorld = pos.xyz;
    
    //pos = mul(pos, view);    
    //pos = mul(pos, projection);

    //output.posProj = pos;
    //output.texcoord = input.texcoord;
    

    //output.color = float3(1.0, 1.0, 0.0) * (1.0 - t) + float3(1.0, 0.0, 0.0) * t;

    return output;
}
