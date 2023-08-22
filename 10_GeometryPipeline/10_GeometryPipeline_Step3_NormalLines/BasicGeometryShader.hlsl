
cbuffer BasicGeometryConstantData : register(b0)
{
    matrix modelWorld;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

cbuffer NormalGeometryConstantData : register(b1)
{
    float scale; // �׷����� ������ ���� ����
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION; //�� ��ǥ���� ��ġ position
    float3 normalWorld : NORMAL; // �� ��ǥ���� normal    
};


struct PixelShaderInput
{
    float4 pos : SV_POSITION; //�� ��ǥ���� ��ġ position
    float3 color : COLOR; // �� ��ǥ���� normal    
};


[maxvertexcount(2)]
void main(point GeometryShaderInput input[1], inout LineStream<PixelShaderInput> outputStream)
{
    PixelShaderInput output;
    float4 posWorld = mul(input[0].posModel, modelWorld);
    float4 normalModel = float4(input[0].normalWorld,  0.0f);
    float4 normalWorld = mul(normalModel, invTranspose);
    normalWorld = float4(normalize(normalWorld.xyz), 0.0);
    
    output.pos = mul(posWorld, view);
    output.pos = mul(output.pos, projection);
    output.color = float3(1.0, 0.0, 0.0);
    outputStream.Append(output);
    
    output.pos = mul(posWorld + normalWorld * scale, view);
    output.pos = mul(output.pos, projection);
    output.color = float3(1.0, 0.0, 1.0);
    outputStream.Append(output);
    

}
