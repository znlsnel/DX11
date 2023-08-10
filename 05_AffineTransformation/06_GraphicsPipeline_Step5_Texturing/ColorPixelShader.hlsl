Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

cbuffer PixelShaderConstantBuffer : register(b0) { float xSplit; };

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET {

    return input.texcoord.x > xSplit
               ? g_texture0.Sample(g_sampler, input.texcoord)
               : float4(1.0, 0.0, 0.0, 1.0);
}
