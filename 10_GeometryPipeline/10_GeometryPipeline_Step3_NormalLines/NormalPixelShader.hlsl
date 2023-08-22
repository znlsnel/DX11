struct PixelShaderInput
{
    float4 pos : SV_POSITION; //�� ��ǥ���� ��ġ position
    float3 color : COLOR; // �� ��ǥ���� normal    
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}