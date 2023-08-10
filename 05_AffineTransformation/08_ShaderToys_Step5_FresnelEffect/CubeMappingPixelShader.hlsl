#include "Common.hlsli" // ���̴������� include ��� ����

TextureCube g_diffuseCube : register(t0);
TextureCube g_specularCube : register(t1);
SamplerState g_sampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // ����: �ؽ��� ��ǥ�� float3 �Դϴ�.
    // ����: error X4532: texlod not supported on this target -> ���̴� ��(����) 5_X�� �ø���
    return g_specularCube.Sample(g_sampler, input.posWorld.xyz);
}