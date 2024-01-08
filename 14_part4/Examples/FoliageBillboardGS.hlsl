#include "common.hlsli"

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
};

struct BillboardPixelShaderInput
{
    float4 pos : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float3 normalWorld : NORMAL0;
    float3 tangentWorld : TANGENT0; 
    float3 posModel : POSITION1; // Volume casting 시작점 
    float2 texCoord : TEXCOORD;
};
 
 
[maxvertexcount(8)]
void main(point GeometryShaderInput input[1], uint primID
     : SV_PrimitiveID, inout TriangleStream<BillboardPixelShaderInput> outputStream)
{
      
    float4 up = float4(0, 1, 0, 0);
    float4 front = mul(float4(0.0, 0.0, -1.0, 0.0), world);
    front = normalize(front);
     
    float4 right = float4(cross(up.xyz, normalize(front.xyz)), 0.0);
     
    BillboardPixelShaderInput output;
    output.tangentWorld =right; 
    {
        output.posWorld = input[0].pos - (right);
        output.pos = output.posWorld;
        output.pos = mul(output.pos, view);
        output.pos = mul(output.pos, proj);
        output.normalWorld = front;
        output.posModel = -(right);
        output.texCoord = float2(1.0, 1.0);

        outputStream.Append(output);

        output.posWorld = input[0].pos - (right) + (0.2 * up);
        output.pos = output.posWorld;
        output.pos = mul(output.pos, view);
        output.pos = mul(output.pos, proj);
        output.normalWorld = front;
        output.posModel = -(right) + (0.2 * up);
        output.texCoord = float2(1.0, 0.0);

        outputStream.Append(output);

        output.posWorld = input[0].pos + (right);
        output.pos = output.posWorld;
        output.pos = mul(output.pos, view);
        output.pos = mul(output.pos, proj);
        output.normalWorld = front;
        output.posModel = +(right);
        output.texCoord = float2(0.0, 1.0);

        outputStream.Append(output);

        output.posWorld = input[0].pos + (right) + (0.2 * up);
        output.pos = output.posWorld;
        output.pos = mul(output.pos, view);
        output.pos = mul(output.pos, proj);
        output.normalWorld = front;
        output.posModel = +(right) + (0.2 * up);
        output.texCoord = float2(0.0, 0.0);

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
    

  