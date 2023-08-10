// TODO: 받아올 constant 선언

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    // TODO: 버텍스 쉐이더와 맞춰주기 (텍스춰 좌표 추가)
};

float4 main(PixelShaderInput input) : SV_TARGET {

    // TODO: 텍스춰 좌표를 이용해서 색 결정

    // Use the interpolated vertex color
    return float4(input.color, 1.0);
}
