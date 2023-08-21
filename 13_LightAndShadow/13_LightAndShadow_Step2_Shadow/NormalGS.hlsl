cbuffer BasicVertexConstantData : register(b0)
{
    matrix modelWorld;
    matrix invTranspose;
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

cbuffer BasicPixelConstData : register(b1)
{
    matrix viewProj;
    float3 eyeWorld;
    float dummy1;
};

cbuffer NormalVertexConstantData : register(b2)
{
    float scale; // 그려지는 선분의 길이 조절
};

struct GeometryShaderInput
{
    float4 posModel : SV_POSITION;
    float3 normalModel : NORMAL;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
};

[maxvertexcount(2)]
void main(point GeometryShaderInput input[1], inout LineStream<PixelShaderInput> outputStream)
{
    PixelShaderInput output;
    
    float4 posWorld = mul(input[0].posModel, modelWorld);
    float4 normalModel = float4(input[0].normalModel, 0.0);
    float4 normalWorld = mul(normalModel, invTranspose);
    normalWorld = float4(normalize(normalWorld.xyz), 0.0);
    
    output.pos = mul(posWorld, viewProj);
    output.color = float3(1.0, 1.0, 0.0);
    outputStream.Append(output);
    
    output.pos = mul(posWorld + 0.1 * scale * normalWorld, viewProj);
    output.color = float3(1.0, 0.0, 0.0);
    outputStream.Append(output);
}
