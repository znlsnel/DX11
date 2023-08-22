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
    float scale; // �׷����� ������ ���� ����
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION; //�� ��ǥ���� ��ġ position
    float3 normalWorld : NORMAL; // �� ��ǥ���� normal    
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    output.posModel = float4(input.posModel, 1.0);
    output.normalWorld = input.normalModel;
    return output;
    //float4 pos = float4(input.posModel, 1.0f);

    //// Normal ���� ��ȯ
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