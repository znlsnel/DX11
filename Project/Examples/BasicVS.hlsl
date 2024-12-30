#include "Common.hlsli" // ���̴������� include ��� ����

// Vertex Shader������ �ؽ��� ���
Texture2D g_heightTexture : register(t0);

 
PixelShaderInput main(VertexShaderInput input)
{
    // �� ��ǥ��� NDC�̱� ������ ���� ��ǥ�� �̿��ؼ� ���� ���
    
    PixelShaderInput output;
    
  
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
    
    uint indices[8]; // ��Ʈ: �� ���!
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    indices[4] = input.boneIndices1.x;
    indices[5] = input.boneIndices1.y;
    indices[6] = input.boneIndices1.z;
    indices[7] = input.boneIndices1.w;

    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    
    // Uniform Scaling ����
    // (float3x3)boneTransforms ĳ�������� Translation ����
    for(int i = 0; i < 8; ++i)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0), boneTransforms[indices[i]]);
        normalModel += weights[i] * mul(input.normalModel, (float3x3)boneTransforms[indices[i]]);
        tangentModel += weights[i] * mul(input.tangentModel, (float3x3)boneTransforms[indices[i]]);
    }

    input.posModel = posModel;
    input.normalModel = normalModel;
    input.tangentModel = tangentModel;

#endif
    
    output.posModel = input.posModel;
    output.normalWorld = mul(float4(input.normalModel, 0.0f), worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    output.posWorld = mul(float4(input.posModel, 1.0f), world).xyz;
     
    //����: windTrunk, windLeaves �ɼǵ� skinnedMeshó�� ��ũ�� ��� ����
    float dist = length(output.posWorld - eyeWorld);
    if (windTrunk != 0.0) 
    { 
        float tempWindT = windTrunk;
        float2 rotCenter = float2(0.0f, -0.5f);
        float2 temp = (input.posModel.xy - rotCenter);
        float coeff = tempWindT * pow(max(0, temp.y), 2.0) * sin(globalTime);
         
        float2x2 rot = float2x2(cos(coeff), sin(coeff), -sin(coeff), cos(coeff));
        
        input.posModel.xy = mul(temp, rot);
        input.posModel.xy += rotCenter;
    }
         
    if (useARTTexture)
    { 
     //   float speed = globalTime * 0.015;
  //      float tempWind = 0.1;
    //    float3 windVel = float3(sin(input.posModel.x * 100.0 + speed * 10)
              //                  * cos(input.posModel.z * 100 + speed * 10), 0, 0)
          //                      * sin(speed * 100.0);
        
  //      float3 coeff = (1.0 - input.texcoord.y) * tempWind * 
  //             dot(input.normalModel, windVel) * input.normalModel;

//        input.posModel.xyz += coeff;
        
    float speed = globalTime * 0.7;
    float tempWind = 0.1;
    // �ٶ��� ����� �ӵ��� �����ϰ� ����
    float3 windVel = float3(sin(speed * 2.0), 0, cos(speed * 2.0)) * tempWind;
 
    // Ǯ�� ���̿� ���� ��鸲�� ������ ����
    float tempY = (1.0 - input.texcoord.y) ;
  
    if (tempY < 1.0 / 3.1){
     
    }
    else if (tempY <  2.0 / 3.1){
       tempY -=1.0 / 3.0;
    }    
    else {
       tempY -= 2.0 / 3.0;
    }
     
    tempY /= 1.0 / 3.0;
    
    tempY -= 0.3;
    tempY = max(0, tempY);   
    tempY /= 0.7;

    float heightFactor = pow(tempY, 2.0); // �Ѹ� �κ��� ���� �������� �ʰ�
 
    // �ٶ� ����� Ǯ�� ���⿡ ���� ���� ��� ���
    float3 coeff = heightFactor * dot(input.normalModel, windVel) * input.normalModel;

    // Ǯ�� ��ġ�� ������Ʈ�Ͽ� ��鸲 ����
        input.posModel.xyz += coeff;
 
 
    }
    
    if (windLeaves != 0.0)
    {
        float3 windVel = float3(sin(input.posModel.x * 100.0 + globalTime * 0.1)
                                * cos(input.posModel.y * 100 + globalTime * 0.1), 0, 0)
                                * sin(globalTime * 10.0);
        
        float3 coeff = (1.0 - input.texcoord.y) * windLeaves * dot(input.normalModel, windVel) * input.normalModel;
        
        input.posModel.xyz += coeff;
    }
      
    //if (useARTTexture)
    //{
    //    input.texcoord
    //}
    
    output.posModel = input.posModel;
    output.normalWorld = mul(float4(input.normalModel, 0.0f), worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    output.posWorld = mul(float4(input.posModel, 1.0f), world).xyz;
     
    if (useHeightMap)
    {
        float height = g_heightTexture.SampleLevel(linearClampSampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        output.posWorld += output.normalWorld * height * heightScale;
    }

    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    output.texcoord = input.texcoord;
    output.tangentWorld = mul(float4(input.tangentModel, 0.0f), world).xyz;
       
    return output;
}
 