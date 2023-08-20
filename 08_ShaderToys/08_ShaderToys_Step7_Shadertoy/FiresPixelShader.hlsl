//////////////////////
// Fire Flame shader

// procedural noise from IQ

cbuffer SamplingPixelConstantData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float iTime;
    float dummy[3];
};


struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};



float2 hash(float2 p)
{
    p = float2(dot(p, float2(127.1, 311.7)),
			 dot(p, float2(269.5, 183.3)));
    return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}

float noise(float2 p)
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;
	
    float2 i = floor(p + (p.x + p.y) * K1);
	
    float2 a = p - i + (i.x + i.y) * K2;
    float2 o = (a.x > a.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    float2 b = a - o + K2;
    float2 c = a - 1.0 + 2.0 * K2;
	
    float3 h = max(0.5 - float3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	
    float3 n = h * h * h * h * float3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
	
    return dot(n, float3(70.0, 70.0, 70.0));
}

float fbm(float2 uv)
{
    float f;
    float2x2 m = float2x2(1.6, 1.2, -1.2, 1.6);
    f = 0.5000 * noise(uv);
    uv = mul(m, uv);
    f += 0.2500 * noise(uv);
    uv = mul(m, uv);
    f += 0.1250 * noise(uv);
    uv = mul(m, uv);
    f += 0.0625 * noise(uv);
    uv = mul(m, uv);
    f = 0.5 + 0.5 * f;
    return f;
}

// no defines, standard redish flames
//#define BLUE_FLAME
//#define GREEN_FLAME

float4 main(PixelShaderInput input) : SV_TARGET
{
    float2 uv = input.texcoord;
    uv.y = 1.0 - uv.y;
    float2 q = uv;
    q.x *= 5.;
    q.y *= 2.;
    float strength = floor(q.x + 1.);
    float T3 = max(3., 1.25 * strength) * iTime;
    q.x = fmod(q.x, 1.0) - 0.5;
    q.y -= 0.25;
    float n = fbm(strength * q - float2(0, T3));
    float c = 1. - 16. * pow(max(0., length(q * float2(1.8 + q.y * 1.5, .75)) - n * max(0., q.y + .25)), 1.2);
//	float c1 = n * c * (1.5-pow(1.25*uv.y,4.));
    float c1 = n * c * (1.5 - pow(2.50 * uv.y, 4.));
    c1 = clamp(c1, 0., 1.);

    float3 col = float3(1.5 * c1, 1.5 * c1 * c1 * c1, c1 * c1 * c1 * c1 * c1 * c1);
	
#ifdef BLUE_FLAME
	col = col.zyx;
#endif
#ifdef GREEN_FLAME
	col = 0.85*col.yxz;
#endif
	
    float a = c * (1. - pow(uv.y, 3.));
    return float4(lerp(float3(0.0, 0.0, 0.0), col, a), 1.0);
}