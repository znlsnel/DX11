#include "common.hlsli"

// Geometry-Shader Object
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-geometry-shader

// Stream-Output Object
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-so-type

cbuffer BillboardContsts : register(b3)
{
    float widthWorld;
    float3 dirWorld;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
};

struct BillboardPixelShaderInput
{
    float4 pos : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float4 center : POSITION1;
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

[maxvertexcount(4)]
void main(point GeometryShaderInput input[1], uint primID : SV_PrimitiveID,
                              inout TriangleStream<BillboardPixelShaderInput> outputStream)
{
    float hw = 0.5 * widthWorld;
    
    //float4 up = float4(0.0, 1.0, 0.0, 0.0);
    float4 up = mul(float4(0, 1, 0, 0), invView); // <- ���� �����͸� ����� ��ȯ (���̾�� ������ ���� ���)
    up.xyz = normalize(up.xyz);
    float4 front = float4(eyeWorld, 1.0) - input[0].pos;
    front.w = 0.0; // ����
    
    // �����尡 ������ �ٶ󺸴� ���� �������� ������
    // �������� �����带 �ٶ󺸴� ���⿡���� ���� (�ؽ��� ��ǥ ����)
    float4 right = float4(cross(up.xyz, normalize(front.xyz)), 0.0);
    
    BillboardPixelShaderInput output;
    
    output.center = input[0].pos; // �������� �߽�
    
    output.posWorld = input[0].pos - hw * right - hw * up;
    output.pos = output.posWorld;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(1.0, 1.0);
    output.primID = primID;
    
    outputStream.Append(output);

    output.posWorld = input[0].pos - hw * right + hw * up;
    output.pos = output.posWorld;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(1.0, 0.0);
    output.primID = primID; // ����
    
    outputStream.Append(output);
    
    output.posWorld = input[0].pos + hw * right - hw * up;
    output.pos = output.posWorld;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(0.0, 1.0);
    output.primID = primID; // ����
    
    outputStream.Append(output);
    
    output.posWorld = input[0].pos + hw * right + hw * up;
    output.pos = output.posWorld;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(0.0, 0.0);
    output.primID = primID; // ����
    
    outputStream.Append(output);

    // ����: GS�� Triangle Strips���� ����մϴ�.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/triangle-strips

    outputStream.RestartStrip(); // Strip�� �ٽ� ����
}
