Texture2D g_texArray : register(t0);
SamplerState g_sampler : register(s0);

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

float4 main(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
