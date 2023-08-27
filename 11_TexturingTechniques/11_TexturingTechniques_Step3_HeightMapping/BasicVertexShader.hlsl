#include "Common.hlsli" // ���̴������� include ��� ����

// Vertex Shader������ �ؽ��� ���
Texture2D g_heightTexture : register(t0);
SamplerState g_sampler : register(s0);

cbuffer BasicVertexConstantData : register(b0)
{
    matrix modelWorld;
    matrix invTranspose;
    matrix view;
    matrix projection;
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

PixelShaderInput main(VertexShaderInput input)
{
    // ��(Model) ����� �� �ڽ��� �������� 
    // ���� ��ǥ�迡���� ��ġ�� ��ȯ�� �����ݴϴ�.
    // �� ��ǥ���� ��ġ -> [�� ��� ���ϱ�] -> ���� ��ǥ���� ��ġ
    // -> [�� ��� ���ϱ�] -> �� ��ǥ���� ��ġ -> [�������� ��� ���ϱ�]
    // -> ��ũ�� ��ǥ���� ��ġ
    
    // �� ��ǥ��� NDC�̱� ������ ���� ��ǥ�� �̿��ؼ� ���� ���
    
    PixelShaderInput output;
    
    // Normal ���� ���� ��ȯ (Height Mapping)
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    // Tangent ���ʹ� modelWorld�� ��ȯ
    float4 tangentWorld = float4(input.tangentModel, 0.0f);
    tangentWorld = mul(tangentWorld, modelWorld);

    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, modelWorld);
    
    if (useHeightMap)
    {
        // VertexShader������ SampleLevel ���
        float height = g_heightTexture.SampleLevel(g_sampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        pos += float4(output.normalWorld * heightScale * height, 0.0) ;

    }

    output.posWorld = pos.xyz; // ���� ��ġ ���� ����

    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.tangentWorld = tangentWorld.xyz;

    output.color = float3(0.0f, 0.0f, 0.0f);

    return output;
}
