cbuffer BillboardPointsConstantData : register(b0)
{
    float3 eyeWorld; // For geometry shader
    float width; // For geometry shader
    Matrix model; // For vertex shader
    Matrix view; // For vertex shader
    Matrix proj; // For vertex shader
};

struct VertexShaderInput
{
    float4 pos : POSITION; // �� ��ǥ���� ��ġ position
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION; // Screen position
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    
    // Geometry shader�� �״�� �Ѱ��ݴϴ�.
    output.pos = input.pos;
    
    return output;
}