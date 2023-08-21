Texture2DArray g_texArray : register(t0);
SamplerState g_sampler : register(s0);

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float3 uvw = float3(input.texCoord, float(input.primID % 5));
    float4 pixelColor = g_texArray.Sample(g_sampler, uvw);

    // clip(x)���� x�� 0���� ������ �� �ȼ��� ���� ������.     
    
    // alpha ���� �ִ� �̹������� �������� 0.9���� ������ clip
    clip(pixelColor.a - 0.9f);
    
    // �ȼ��� ���� ����� ����� ��� ���̸� clip
    //TODO: clip(...)
    float i = (pixelColor.r + pixelColor.g + pixelColor.b) / 3.0;
    clip((i > 0.7) ? -1 : 1);
    
    //clip((pixelColor.a < 0.9f) || (pixelColor.r + pixelColor.g + pixelColor.b) > 2.4 ? -1 : 1);
    
    PixelShaderOutput output;
    
    output.pixelColor = pixelColor;

    return output;
}
