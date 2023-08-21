#include "Common.hlsli" // ���̴������� include ��� ����

cbuffer MeshConstants : register(b0)
{
    matrix world; // Model(�Ǵ� Object) ��ǥ�� -> World�� ��ȯ
    matrix worldIT; // World�� InverseTranspose
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

float4 main(VertexShaderInput input) : SV_POSITION
{
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    return mul(pos, viewProj);
}
