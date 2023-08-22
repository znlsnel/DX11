struct PixelShaderInput
{
    float4 pos : SV_POSITION; //¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ position
    float3 color : COLOR; // ¸ðµ¨ ÁÂÇ¥°èÀÇ normal    
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}