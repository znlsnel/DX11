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
    float3 posModel : POSITION1; // Volume casting 시작점 
};

cbuffer foliageBuffer : register(b4)
{
    uint foliageID;
    uint3 dummy;
}
//struct PixelShaderInput
//{
//    float4 posProj : SV_POSITION; // Screen position
//    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
//    float3 normalWorld : NORMAL0;
//    float2 texcoord : TEXCOORD0;
//    float3 tangentWorld : TANGENT0;
//    float3 posModel : POSITION1; // Volume casting 시작점 
//};

 
[maxvertexcount(8)]
void main(point GeometryShaderInput input[1], uint primID
     : SV_PrimitiveID, inout TriangleStream<BillboardPixelShaderInput> outputStream)
{
    
    float4 up = float4(0, 1, 0, 0);
    float4 front = mul(float4(0.0, 0.0, -1.0, 0.0), world);
    
    front = float4(eyeWorld - input[0].pos.xyz, 0.0);
    front = normalize(front); 
    
    float4 right = float4(cross(up.xyz, normalize(front.xyz)), 0.0);
    right = normalize(right) * 0.2;
    BillboardPixelShaderInput output;
    
    float yOffset = float(foliageID + 1) / 6.0;
    output.tangentWorld =right; 
    {
        output.posWorld = input[0].pos - (right); 
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = -(right); 
        output.texCoord = float2(1.0, yOffset);
         
        outputStream.Append(output);

        output.posWorld = input[0].pos - (right) + (0.2 * up);
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = -(right) + (0.2 * up);
        output.texCoord = float2(1.0, yOffset - (1.0/6.0));

        outputStream.Append(output);

        output.posWorld = input[0].pos + (right);
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = +(right);
        output.texCoord = float2(0.0, yOffset);

        outputStream.Append(output);

        output.posWorld = input[0].pos + (right) + (0.2 * up);
        output.posProj = output.posWorld;
        output.posProj = mul(output.posProj, view);
        output.posProj = mul(output.posProj, proj);
        output.normalWorld = front;
        output.posModel = +(right) + (0.2 * up);
        output.texCoord = float2(0.0, yOffset - (1.0 / 6.0));
          
        outputStream.Append(output);
    }
    //{
        
    //    output.posWorld = input[0].pos - (front);
    //    output.pos = output.posWorld;
    //    output.pos = mul(output.pos, view);
    //    output.pos = mul(output.pos, proj);
    //    output.texCoord = float2(1.0, 1.0);
    //    output.normalWorld = right;
    //    output.posModel = -(front);
    //    outputStream.Append(output);

    //    output.posWorld = input[0].pos - (front) + (0.2 * up);
    //    output.pos = output.posWorld;
    //    output.pos = mul(output.pos, view);
    //    output.pos = mul(output.pos, proj);
    //    output.texCoord = float2(1.0, 0.0);
    //    output.normalWorld = right;
    //    output.posModel = -(front) + (0.2 * up);

    //    outputStream.Append(output);

    //    output.posWorld = input[0].pos + (front);
    //    output.pos = output.posWorld;
    //    output.pos = mul(output.pos, view);
    //    output.pos = mul(output.pos, proj);
    //    output.texCoord = float2(0.0, 1.0);
    //    output.normalWorld = right;
    //    output.posModel = +(front);
    //    outputStream.Append(output);

    //    output.posWorld = input[0].pos + (front) + (0.2 * up);
    //    output.pos = output.posWorld;
    //    output.pos = mul(output.pos, view);
    //    output.pos = mul(output.pos, proj);
    //    output.texCoord = float2(0.0, 0.0);
    //    output.normalWorld = right;
    //    output.posModel = +(front) + (0.2 * up);
    //    outputStream.Append(output);
    //}
    // 주의: GS는 Triangle Strips으로 출력합니다.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/triangle-strips
     
    outputStream.RestartStrip(); // Strip을 다시 시작
}
    

  