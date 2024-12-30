#include "common.hlsli"

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
};

struct BillboardPixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float3 normalWorld : NORMAL0;
    float2 texCoord : TEXCOORD; 
    float3 tangentWorld : TANGENT0; 
    float3 posModel : POSITION1; // Volume casting ������ 
};

cbuffer foliageBuffer : register(b4)
{
    uint foliageID;
    uint3 dummy;
}
//struct PixelShaderInput
//{
//    float4 posProj : SV_POSITION; // Screen position
//    float3 posWorld : POSITION0; // World position (���� ��꿡 ���)
//    float3 normalWorld : NORMAL0;
//    float2 texcoord : TEXCOORD0;
//    float3 tangentWorld : TANGENT0;
//    float3 posModel : POSITION1; // Volume casting ������ 
//};

 
[maxvertexcount(8)]
void main(point GeometryShaderInput input[1], uint primID
     : SV_PrimitiveID, inout TriangleStream<BillboardPixelShaderInput> outputStream)
{
    
    float4 up = float4(0, 1, 0, 0);
    float3 reflectPos = mul(float4(input[0].pos.xyz, 0.0), reflectWorld).xyz;
      
    float4 front = float4(eyeWorld - reflectPos, 0.0); 
    front = normalize(front); 
    
    float4 right = float4(cross(up.xyz, normalize(front.xyz)), 0.0);
    right = normalize(right) * 0.3;
    BillboardPixelShaderInput output;
     
    float size = 1; 
    right *= size;
    up *= size;
    
    float yOffset = float(foliageID + 1) / 6.0;
    output.tangentWorld =right; 
    {
        output.posWorld = input[0].pos - (right); 
        output.posWorld *= size;
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = -(right); 
        output.texCoord = float2(1.0, yOffset);
         
        outputStream.Append(output);

        output.posWorld = input[0].pos - (right) + (0.15 * up);
        output.posWorld *= size;
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = -(right) + (0.15 * up);
        output.texCoord = float2(1.0, yOffset - (1.0/6.0));

        outputStream.Append(output);

        output.posWorld = input[0].pos + (right);
        output.posWorld *= size;
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = +(right);
        output.texCoord = float2(0.0, yOffset);

        outputStream.Append(output); 
          
        output.posWorld = input[0].pos + (right) + (0.15 * up);
        output.posWorld *= size;
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = +(right) + (0.15 * up);
        output.texCoord = float2(0.0, yOffset - (1.0 / 6.0));
          
        outputStream.Append(output);
    }

    outputStream.RestartStrip(); // Strip�� �ٽ� ����
}
    

  