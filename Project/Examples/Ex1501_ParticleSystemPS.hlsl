struct PixelShaderInput
{
    float4 pos : SV_POSITION; // not POSITION
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    uint primID : SV_PrimitiveID;
};

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

// �Ϲ������� Sprite�� �ؽ��縦 ���� ����մϴ�.
// �� ����ó�� �������� ������ ���� ���� �ֽ��ϴ�.
float4 main(PixelShaderInput input) : SV_TARGET
{
    float dist = length(float2(0.5, 0.5) - input.texCoord) * 2;
    float scale = smootherstep(1 - dist);
    return float4(input.color.rgb * scale, 1);
}
