#include "Common.hlsli" // ���̴������� include ��� ����

struct DepthOnlyPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

DepthOnlyPixelShaderInput main(VertexShaderInput input) 
{

#ifdef SKINNED
    
    // ���� �ڷ�: Luna DX 12 ����
    
    float weights[8];
    weights[0] = input.boneWeights0.x;
    weights[1] = input.boneWeights0.y;
    weights[2] = input.boneWeights0.z;
    weights[3] = input.boneWeights0.w;
    weights[4] = input.boneWeights1.x;
    weights[5] = input.boneWeights1.y;
    weights[6] = input.boneWeights1.z;
    weights[7] = input.boneWeights1.w;
    
    uint indices[8];
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    indices[4] = input.boneIndices1.x;
    indices[5] = input.boneIndices1.y;
    indices[6] = input.boneIndices1.z;
    indices[7] = input.boneIndices1.w;

    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    
    // Uniform Scaling ���� (worldIT ���ʿ�)
    // (float3x3) ĳ�������� Translation ����
    for(int i = 0; i < 8; ++i)
    {
        // TODO: 
    posModel += weights[i] * mul(float4(input.posModel, 1.0), boneTransforms[indices[i]]);
    }

     input.posModel = posModel;

#endif
    
    float3 posWorld = mul(float4(input.posModel, 1.0), world).xyz;
    float dist = length(posWorld - eyeWorld);
    if (windTrunk != 0.0)
    { 
        float2 rotCenter = float2(0.0f, -0.5f);
        float2 temp = (input.posModel.xy - rotCenter);
        float coeff = windTrunk * pow(max(0, temp.y), 2.0) * sin(globalTime);
        
        float2x2 rot = float2x2(cos(coeff), sin(coeff), -sin(coeff), cos(coeff));
        
        input.posModel.xy = mul(temp, rot);
        input.posModel.xy += rotCenter;
    }
    
    
    if (windLeaves != 0.0)
    {
        float3 windVel = float3(sin(input.posModel.x * 100.0 + globalTime * 0.1)
                                * cos(input.posModel.y * 100 + globalTime * 0.1), 0, 0)
                                * sin(globalTime * 10.0);
        
        float3 coeff = (1.0 - input.texcoord.y) * windLeaves * dot(input.normalModel, windVel) * input.normalModel;
        
        input.posModel.xyz += coeff;
    } 
    DepthOnlyPixelShaderInput result;
    
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    result.posProj = mul(pos, viewProj);
    result.texcoord = input.texcoord;
    return result;
}
