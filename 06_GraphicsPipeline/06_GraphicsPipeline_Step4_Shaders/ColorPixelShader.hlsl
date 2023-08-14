// TODO: 받아올 constant 선언

cbuffer PixelShaderConstantBuffer : register(b0) { float xSplit; }

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD0;

    // TODO: 버텍스 쉐이더와 맞춰주기 (텍스춰 좌표 추가)
};

float4 main(PixelShaderInput input) : SV_TARGET {

    // TODO: 텍스춰 좌표를 이용해서 색 결정

    // Use the interpolated vertex color

    return input.texcoord.x > xSplit ? float4(0.0, 0.0, 1.0, 1.0)
                                     : float4(1.0, 0.0, 0.0, 1.0);
}
