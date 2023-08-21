struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    uint primID : SV_PrimitiveID;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    output.pixelColor = (input.primID % 2 == 0) ?
        float4(1.0f, 1.0f, 1.0f, 1.0f) : float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return output;
}
