Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
SamplerState g_sampler : register(s0);

cbuffer SamplingPixelConstantData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float4 options;
};

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    // Compute Shader X
    float3 originTexture = g_texture0.Sample(g_sampler, input.texcoord).xyz;
    float3 gausianTexture = g_texture1.Sample(g_sampler, input.texcoord).xyz * strength;

    return float4(originTexture + gausianTexture, 1.0);
}