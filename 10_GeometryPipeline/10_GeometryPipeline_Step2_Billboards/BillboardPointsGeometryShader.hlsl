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
    float2 texCoord : TEXCOORD; // ÅØ½ºÃç ÁÂÇ¥ Ãß°¡
    uint primID : SV_PrimitiveID;
};

[maxvertexcount(4)]
void main(point GeometryShaderInput input[1], uint primID : SV_PrimitiveID,
                              inout TriangleStream<PixelShaderInput> outputStream)
{
    float hw = 0.5 * width;
    
    PixelShaderInput output;
    float4 front = float4(normalize(eyeWorld - input[0].pos.xyz), 0.0);
    float4 up = float4(0.0, 1.0, 0.0, 0.0);
    float4 right = float4(normalize(cross(front.xyz, up.xyz)), 0.0);
    
    output.pos = input[0].pos - (right * hw) - (up * hw);

    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);

    output.texCoord = float2(0.0, 1.0);
    output.primID = primID;
    
    outputStream.Append(output);

    output.pos = input[0].pos - (right * hw) + (up * hw);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(0.0, 0.0);
    output.primID = primID;
    
    outputStream.Append(output);
    
    output.pos = input[0].pos + (right * hw) - (up * hw);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(1.0, 1.0);
    output.primID = primID;
    
    outputStream.Append(output);
    
    output.pos = input[0].pos + (right * hw) + (up * hw);;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, proj);
    output.texCoord = float2(1.0, 0.0);
    output.primID = primID;
    
    outputStream.Append(output);

    // outputStream.RestartStrip();
}