cbuffer SamplingPixelConstantData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float iTime;
    float2 iResolution;
    float dummy;
};


struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


#define R iResolution.xy
#define ss(a,b,t) smoothstep(a,b,t)

float gyroid(float3 seed)
{
    return dot(sin(seed), cos(seed.yzx));
}

float fbm(float3 seed)
{
    float result = 0., a = .5;
    for (int i = 0; i < 6; ++i, a /= 2.)
    {
        seed.x += iTime * .01 / a;
        seed.z += result * .5;
        result += gyroid(seed / a) * a;
    }
    return result;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float2 p = (2. * (input.texcoord * R) - R) / R.y;
    p.y *= -1.0;
    float count = 2.;
    float shades = 3.;
    float shape = abs(fbm(float3(p * .5, 0.))) - iTime * .1 - p.x * .1;
    float gradient = frac(shape * count + p.x);
    float3 blue = float3(.459, .765, 1.);
    float3 tint = lerp(blue * lerp(.6, .8, gradient), float3(1.0, 1.0, 1.0), round(pow(gradient, 4.) * shades) / shades);
    float3 color = lerp(tint, blue * .2, fmod(floor(shape * count), 2.));
    return float4(color, 1.0);
}