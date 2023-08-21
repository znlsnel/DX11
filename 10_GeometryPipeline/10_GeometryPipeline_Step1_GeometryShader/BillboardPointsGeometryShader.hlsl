// Geometry-Shader Object
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-geometry-shader

// Stream-Output Object
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-so-type

cbuffer BillboardPointsConstantData : register(b0)
{
    float3 eyeWorld;
    float width;
    Matrix model; // For vertex shader
    Matrix view; // For vertex shader
    Matrix proj; // For vertex shader
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // not POSITION
    uint primID : SV_PrimitiveID;
};

//TODO: PointStream -> TriangleStream
[maxvertexcount(100)] // 최대 출력 Vertex 갯수
void main(point GeometryShaderInput input[1], uint primID : SV_PrimitiveID,
                              inout PointStream<PixelShaderInput> outputStream)
{
    
    // float hw = 0.5 * width;
    
    PixelShaderInput output;
    
    output.pos = input[0].pos;
    
    for (int i = 0; i < 100; i ++)
    {
        output.pos = input[0].pos + float4(0.0, 0.003, 0.0, 0.0) * float(i);
        output.pos = mul(output.pos, view);
        output.pos = mul(output.pos, proj);
        output.primID = primID;

        outputStream.Append(output);
    }
 
    // 주의: GS는 Triangle Strips으로 출력합니다.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/triangle-strips

    // outputStream.RestartStrip(); // Strip을 다시 시작
}
