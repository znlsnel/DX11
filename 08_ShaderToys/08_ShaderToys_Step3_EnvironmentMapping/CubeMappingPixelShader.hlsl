#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

TextureCube g_textureCube0 : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // 주의: 텍스춰 좌표가 float3 입니다.
    // 주의: error X4532: texlod not supported on this target -> 쉐이더 모델(버전) 5_X로 올리기
    return g_textureCube0.Sample(g_sampler, input.posWorld.xyz);
}