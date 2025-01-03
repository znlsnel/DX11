#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

// Vertex Shader에서도 텍스춰 사용
Texture2D g_heightTexture : register(t0);

 
PixelShaderInput main(VertexShaderInput input)
{
    // 뷰 좌표계는 NDC이기 때문에 월드 좌표를 이용해서 조명 계산
    
    PixelShaderInput output;
    
  
#ifdef SKINNED
    
    // 참고 자료: Luna DX 12 교재
    
    float weights[8];
    weights[0] = input.boneWeights0.x;
    weights[1] = input.boneWeights0.y;
    weights[2] = input.boneWeights0.z;
    weights[3] = input.boneWeights0.w;
    weights[4] = input.boneWeights1.x;
    weights[5] = input.boneWeights1.y;
    weights[6] = input.boneWeights1.z;
    weights[7] = input.boneWeights1.w;
    
    uint indices[8]; // 힌트: 꼭 사용!
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
    
    // Uniform Scaling 가정
    // (float3x3)boneTransforms 캐스팅으로 Translation 제외
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
     
    //참고: windTrunk, windLeaves 옵션도 skinnedMesh처럼 매크로 사용 가능
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
    // 바람의 방향과 속도를 일정하게 설정
    float3 windVel = float3(sin(speed * 2.0), 0, cos(speed * 2.0)) * tempWind;
 
    // 풀의 높이에 따라 흔들림의 정도를 조정
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

    float heightFactor = pow(tempY, 2.0); // 뿌리 부분이 거의 움직이지 않게
 
    // 바람 방향과 풀의 방향에 따른 힘의 계수 계산
    float3 coeff = heightFactor * dot(input.normalModel, windVel) * input.normalModel;

    // 풀의 위치를 업데이트하여 흔들림 적용
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
 