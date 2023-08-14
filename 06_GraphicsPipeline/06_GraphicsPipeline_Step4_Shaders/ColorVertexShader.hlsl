// Data Types (HLSL)
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-data-types

// Shader Constants (HLSL)
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-constants

// Register
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-register

// float4, matrix
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math

cbuffer ModelViewProjectionConstantBuffer : register(b0) {
    matrix model; // matrix 대신에 float4x4를 사용할 수도 있습니다.
    matrix view;
    matrix projection;
};

// Semantics
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics

struct VertexShaderInput {
    float3 pos : POSITION;
    float3 color : COLOR0;
    float2 texcoord : TEXCOORD0;

    // TODO: 텍스춰 좌표 추가!
};

// SV_POSITION : Interpolation된 정보를 넣음
// PixelShaderInput == VertexShadeOutput
struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD0;

    // TODO: 텍스춰 좌표 추가!
};

// Intrinsic Functions
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions

PixelShaderInput main(VertexShaderInput input) {

    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);

    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.pos = pos;
    output.color = input.color;
    output.texcoord = input.texcoord;
    // TODO: 텍스춰 좌표 추가!

    return output;
}
