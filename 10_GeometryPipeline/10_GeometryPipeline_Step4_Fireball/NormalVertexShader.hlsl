struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION;
    float3 normalWorld : NORMAL;
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;

    output.posModel = float4(input.posModel, 1.0);
    output.normalWorld = input.normalModel;

    return output;
}
