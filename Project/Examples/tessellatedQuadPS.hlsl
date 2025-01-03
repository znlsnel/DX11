#include "Common.hlsli" // 쉐이더에서도 include 사용 가능
#include "DiskSamples.hlsli"

// 참고자료
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2DArray albedoTex : register(t0); 
Texture2DArray normalTex : register(t1);
Texture2DArray ORDpTex : register(t2);

Texture2D textureMap : register(t3);

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
static float2 currTexture = { 0, 0 };
static float2 rightTexture = { 0, 0 };
static float2 upTexture = { 0, 0 };
static float2 upRightTexture = { 0,0};
 

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
    //return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
    float4 indexColor : SV_Target1;
};

float4 GetTexture(Texture2DArray textureArray, SamplerState ss, float2 texc, float lod)
{
    //float4 curT = textureArray.SampleLevel(linearWrapSampler, float3(texc, currTexture.x), lod);
    //float4 rightT = textureArray.SampleLevel(linearWrapSampler, float3(texc, rightTexture.x), lod);
    //float4 upT = textureArray.SampleLevel(linearWrapSampler, float3(texc, upTexture.x), lod);
    //float4 upRightT = textureArray.SampleLevel(linearWrapSampler, float3(texc, upRightTexture.x), lod);
    float4 curT = textureArray.Sample(linearWrapSampler, float3(texc, currTexture.x));
    float4 rightT = textureArray.Sample(linearWrapSampler, float3(texc, rightTexture.x));
    float4 upT = textureArray.Sample(linearWrapSampler, float3(texc, upTexture.x));
    float4 upRightT = textureArray.Sample(linearWrapSampler, float3(texc, upRightTexture.x));
    float4 result = 
    curT * (currTexture.y) +  
    rightT * (rightTexture.y)+ 
    upT * (upTexture.y) + 
    upRightT * (upRightTexture.y);
     
    //result *= 0.5; 
    return result;  
}

float3 GetNormal(PixelShaderInput input, float lod)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
         //float3 normal = normalTex.SampleLevel(linearWrapSampler, float3(input.texcoord, currTexture), lod).rgb;
        float3 normal = GetTexture(normalTex, linearWrapSampler, input.texcoord, lod);
         
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                  float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    //float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;
    float3 irradiance = irradianceIBLTex.Sample(linearWrapSampler, normalWorld).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    //float2 specularBRDF = brdfTex.SampleLevel(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float2 specularBRDF = brdfTex.Sample(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness)).rg;
    float3 specularIrradiance = specularIBLTex.SampleLevel(linearWrapSampler, reflect(-pixelToEye, normalWorld),
                                                            roughness * 10.0f).rgb;
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
                            float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float NdotH, float roughness, float alphaPrime)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    return alphaPrime * alphaPrime / (3.141592 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float SchlickG1(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

// 참고: https://github.com/opengl-tutorials/ogl/blob/master/tutorial16_shadowmaps/ShadowMapping.fragmentshader
float random(float3 seed, int i)
{
    float4 seed4 = float4(seed, i);
    float dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));
    return frac(sin(dot_product) * 43758.5453);
}

// NdcDepthToViewDepth
float N2V(float ndcDepth, matrix invProj)
{
    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
    return pointView.z / pointView.w;
}

#define NEAR_PLANE 0.1
// #define LIGHT_WORLD_RADIUS 0.001
#define LIGHT_FRUSTUM_WIDTH 0.34641 // <- 계산해서 찾은 값

// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT
// #define LIGHT_RADIUS_UV (LIGHT_WORLD_RADIUS / LIGHT_FRUSTUM_WIDTH)

float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
{
    float sum = 0.0f;
    for (int i = 0; i < 64; ++i)
    {
        float2 offset = diskSamples64[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(
            shadowCompareSampler, uv + offset, zReceiverNdc);
    }
    return sum / 64;
}

// void Func(out float a) <- c++의 void Func(float& a) 처럼 출력값 저장 가능

void FindBlocker(out float avgBlockerDepthView, out float numBlockers, float2 uv,
                 float zReceiverView, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
{
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;
    
    float searchRadius = lightRadiusUV * (zReceiverView - NEAR_PLANE) / zReceiverView;

    float blockerSum = 0;
    numBlockers = 0;
    for (int i = 0; i < 64; ++i)
    {
        //float shadowMapDepth =
        //    shadowMap.SampleLevel(shadowPointSampler, float2(uv + diskSamples64[i] * searchRadius), 0).r;
        
        float shadowMapDepth =
            shadowMap.Sample(shadowPointSampler, float2(uv + diskSamples64[i] * searchRadius)).r;
        
        shadowMapDepth = N2V(shadowMapDepth, invProj);
        
        if (shadowMapDepth < zReceiverView)
        {
            blockerSum += shadowMapDepth;
            numBlockers++;
        }
    }
    avgBlockerDepthView = blockerSum / numBlockers;
}

float PCSS(float2 uv, float zReceiverNdc, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
{
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;
    
    float zReceiverView = N2V(zReceiverNdc, invProj);
    
    // STEP 1: blocker search
    float avgBlockerDepthView = 0;
    float numBlockers = 0;

    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, shadowMap, invProj, lightRadiusWorld);

    if (numBlockers < 1)
    {
        // There are no occluders so early out(this saves filtering)
        return 1.0f;
    }
    else
    {
        // STEP 2: penumbra size
        float penumbraRatio = (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
        float filterRadiusUV = penumbraRatio * lightRadiusUV * NEAR_PLANE / zReceiverView;

        // STEP 3: filtering
        return PCF_Filter(uv, zReceiverNdc, filterRadiusUV, shadowMap);
    }
}

float3 LightRadiance(Light light, float3 representativePoint, float3 posWorld, float3 normalWorld, Texture2D shadowMap)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : representativePoint - posWorld; //: light.position - posWorld;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT
                      ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                      : 1.0f;
        
    // Distance attenuation
    float att = saturate((light.fallOffEnd - lightDist)
                         / (light.fallOffEnd - light.fallOffStart));

    // Shadow map
    float shadowFactor = 1.0;

    if (light.type & LIGHT_SHADOW)
    {
        const float nearZ = 0.01; // 카메라 설정과 동일
        
        // 1. Project posWorld to light screen    
        float4 lightScreenOverall = float4(0.0, 0.0, 0.0, 0.0);
        float2 lightTexcoordOverall = float2(0.0, 0.0);
        float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
        lightScreen.xyz /= lightScreen.w;
        
        // 2. 카메라(광원)에서 볼 때의 텍스춰 좌표 계산
        float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
        lightTexcoord += 1.0;
        lightTexcoord *= 0.5;
         
        // 3. 쉐도우맵에서 값 가져오기
        float depth = shadowMap.Sample(shadowPointSampler, lightTexcoord).r;
        bool usedOverallShadowMap = false;
        // 4. 가려져 있다면 그림자로 표시
        if (depth - 0.1 < lightScreen.z)
            shadowFactor = 0.0;
         
        //else if (light.type & LIGHT_DIRECTIONAL)
        //{
        //    lightScreenOverall = mul(float4(posWorld, 1.0), lights[MAX_LIGHTS - 1].viewProj);
        //    lightScreenOverall.xyz /= lightScreenOverall.w;
        
        //// 2. 카메라(광원)에서 볼 때의 텍스춰 좌표 계산
        //    lightTexcoordOverall = float2(lightScreenOverall.x, -lightScreenOverall.y);
        //    lightTexcoordOverall += 1.0;
        //    lightTexcoordOverall *= 0.5;
        //    depth = shadowMaps[MAX_LIGHTS - 1].Sample(shadowPointSampler, lightTexcoordOverall).r;
        //    if (depth + 0.001 < lightScreenOverall.z)
        //    {
        //        shadowFactor = 0.0;
        //        usedOverallShadowMap = true;
        //    }
        //}
        
        if (usedOverallShadowMap == false)
        {
            uint width, height, numMips;
            shadowMap.GetDimensions(0, width, height, numMips);
        
            // float dx = 5.0 / (float) width;
            // shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.001, dx, shadowMap);
        
            float radiusScale = 0.5; // 광원의 반지름을 키웠을 때 깨지는 것 방지
            shadowFactor = PCSS(lightTexcoord, lightScreen.z - 0.001, shadowMap, light.invProj, light.radius * radiusScale);
                   
        }
        else
        {
            uint width, height, numMips;
            shadowMaps[MAX_LIGHTS - 1].GetDimensions(0, width, height, numMips);
        
            // float dx = 5.0 / (float) width;
            // shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.001, dx, shadowMap);
        
            float radiusScale = 0.5; // 광원의 반지름을 키웠을 때 깨지는 것 방지
            shadowFactor = PCSS(lightTexcoordOverall, lightScreenOverall.z - 0.001, shadowMaps[MAX_LIGHTS - 1], light.invProj, light.radius * radiusScale);
                   
        }
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

float3 DrawLight(PixelShaderInput input, Light light, Texture2D shadowMap,
float3 normalWorld, float3 pixelToEye, float4 albedo, float metallic, float roughness, float3 directLighting)
{
    if (light.type == 0)
        return float3(0.0, 0.0, 0.0);
    
    float3 L = light.position - input.posWorld;
    if (light.type & LIGHT_DIRECTIONAL)
        L = -light.direction;
    float3 r = normalize(reflect(eyeWorld - input.posWorld, normalWorld));
    float3 centerToRay = dot(L, r) * r - L;
    float3 representativePoint = L + centerToRay * clamp(light.radius / length(centerToRay), 0.0, 1.0);
    representativePoint += input.posWorld;
    float3 lightVec = representativePoint - input.posWorld;

            //float3 lightVec = lights[i].position - input.posWorld;
    float lightDist = length(lightVec);
    lightVec /= lightDist;
    float3 halfway = normalize(pixelToEye + lightVec);
        
    float NdotI = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotO = max(0.0, dot(normalWorld, pixelToEye));
        
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo.rgb, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 diffuseBRDF = kd * albedo.rgb;

            // Sphere Normalization
    float alpha = roughness * roughness;
    float alphaPrime = saturate(alpha + light.radius / (2.0 * lightDist));

    float D = NdfGGX(NdotH, roughness, alphaPrime);
    float3 G = SchlickGGX(NdotI, NdotO, roughness);
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

    float3 radiance = LightRadiance(light, representativePoint, input.posWorld, normalWorld, shadowMap);
            
            //if (i == 0)
            //    radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMaps[0]);
            //if (i == 1)
            //    radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMap1);
            //if (i == 2)
            //    radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMap2);
            
            // 오류 임시 수정 (radiance가 (0,0,0)일 경우  directLighting += ... 인데도 0 벡터가 되어버림
    if (abs(dot(float3(1, 1, 1), radiance)) > 1e-5) 
        directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;

    return directLighting;
}
 
PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
        
    float texDx = 1.0 / 1024; 
    float texSize = 50.0;
    float2 originTexc = input.texcoord / texSize;
    float2 texc = input.texcoord; 
    { // 0 ~ 1024 -> 0 ~ 1
        texc /= texSize;
        texc *= 1024; 
        texc.x = floor(texc.x);  
        texc.y = floor(texc.y);
        texc /= 1024;
    }
    float2 texDir = originTexc - texc;
  // float2 texDir = texc - originTexc;
    {
        texDir.x = texDir.x > 0 ? texDx : -texDx;
        texDir.y = texDir.y > 0 ? texDx : -texDx;
    } 
        
    currTexture.x = int(textureMap.SampleLevel(pointWrapSampler, texc,0).r * 255);
    float2 rightTexc = texc + float2(texDir.x, 0);
    rightTexture.x = int(textureMap.SampleLevel(pointWrapSampler, rightTexc, 0).r * 255);
    float2 upTexc = texc + float2(0, texDir.y);
    upTexture.x = int(textureMap.SampleLevel(pointWrapSampler, upTexc, 0).r * 255);
    float2 upRightTexc = texc + float2(texDir.x, texDir.y);
    upRightTexture.x = int(textureMap.SampleLevel(pointWrapSampler, upRightTexc, 0).r * 255);
    
    float eyeToPixelLength = length(input.posWorld - eyeWorld);
       
  
        
    float originToTexLength = length(originTexc - texc); 
    float originToRightTexLength = length(originTexc - rightTexc);
    float originToUpTexLength = length(originTexc - upTexc);
    float originToUpRightTexLength = length(originTexc - upRightTexc);
          
    originToTexLength = clamp(originToTexLength - (texDx / 8), 0.0, originToTexLength);
    originToRightTexLength = clamp(originToRightTexLength - (texDx / 8), 0.0, originToRightTexLength);
    originToUpTexLength = clamp(originToUpTexLength - (texDx / 8), 0.0, originToUpTexLength);
    originToUpRightTexLength = clamp(originToUpRightTexLength - (texDx / 8), 0.0, originToUpRightTexLength);
      
     
    
    float totalLength = originToTexLength + originToRightTexLength + 
                                        originToUpTexLength + originToUpRightTexLength;
       
    currTexture.y = (totalLength / originToTexLength);
    rightTexture.y = (totalLength / originToRightTexLength);
    upTexture.y = (totalLength / originToUpTexLength);
    upRightTexture.y = (totalLength / originToUpRightTexLength);

    //currTexture.y = saturate(originToTexLength / totalLength);
    //rightTexture.y = saturate(originToRightTexLength / totalLength);
    //upTexture.y = saturate(originToUpTexLength / totalLength);
    //upRightTexture.y = saturate(originToUpRightTexLength / totalLength);
       
    totalLength = currTexture.y + rightTexture.y + upTexture.y + upRightTexture.y;
    currTexture.y /= totalLength;
    rightTexture.y /= totalLength;
    upTexture.y /= totalLength;
    upRightTexture.y /= totalLength;
        
    currTexture.y = saturate(currTexture.y);
    rightTexture.y = saturate(rightTexture.y);
    upTexture.y = saturate(upTexture.y);
    upRightTexture.y = saturate(upRightTexture.y);


    
    float lod = length(input.posWorld - eyeWorld);
   // lod -= 3;
   //lod;  
    lod = clamp(lod, 0.0, 5.0); 
    
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input, lod);
    
    //float4 albedo = useAlbedoMap ? albedoTex.SampleLevel(linearWrapSampler, float3(input.texcoord, currTexture), lod) * float4(albedoFactor, 1)
    //                             : float4(albedoFactor, 1);
    float4 albedo = float4(albedoFactor, 1);
    if (useAlbedoMap)
    {
        //albedo = albedoTex.SampleLevel(linearWrapSampler, float3(input.texcoord, currTexture), lod) * float4(albedoFactor, 1);
        albedo = GetTexture(albedoTex, linearWrapSampler, input.texcoord, lod) * float4(albedoFactor, 1);
    }
    
    clip(albedo.a - 0.5); // Tree leaves
    
    //float ao = useAOMap ? ORDpTex.SampleLevel(linearWrapSampler, float3(input.texcoord, currTexture.x), lod).r : 1.0;
    //float roughness = useRoughnessMap ? clamp(ORDpTex.SampleLevel(linearWrapSampler, float3(input.texcoord, currTexture.x), lod).g, minRoughness, 1.0) : roughnessFactor;
    float metallic = metallicFactor; 
    float3 emission = emissionFactor;
    float ao = useAOMap ? ORDpTex.Sample(linearWrapSampler, float3(input.texcoord, currTexture.x)).r : 1.0;
    float roughness = useRoughnessMap ? clamp(ORDpTex.Sample(linearWrapSampler, float3(input.texcoord, currTexture.x)).g, minRoughness, 1.0) : roughnessFactor;
      
    float3 ambientLighting = AmbientLightingByIBL(albedo.rgb, normalWorld, pixelToEye, ao, metallic, roughness) * strengthIBL;
    
    float3 directLighting = float3(0, 0, 0);

    // 0.3, 1.0, 3.0, 5.0, 10.0
    float distance_worldToEye = length(eyeWorld - input.posWorld);

    int distanceID = 0;
    float distances[7] = { 0.0, 0.3, 1.0, 3.0, 5.0, 10.0, 100.0 };
     
    [unroll]
    for (int k = 0; k < 6; ++k)
    {
        if (distance_worldToEye >= distances[k] && distance_worldToEye < distances[k + 1])
        {
            distanceID = k;
            break;
        }
    }
      
    if (distanceID == 0)
    {
        directLighting = DrawLight(input, lights[0], shadowMaps[0],
        normalWorld, pixelToEye, albedo, metallic, roughness, directLighting);
    }
    else if (distanceID == 5)
    {
        directLighting = DrawLight(input, lights[4], shadowMaps[4],
        normalWorld, pixelToEye, albedo, metallic, roughness, directLighting);
        
    }
    else
    {
        float3 tempDL[2] =
        {
            float3(0.0, 0.0, 0.0),
            float3(0.0, 0.0, 0.0)
        }; 
        // 임시로 unroll 사용
        // warning X3550: sampler array index must be a literal expression, forcing loop to unroll

        [unroll]
        for (int i = 0; i <= 4; i++)
        {
            if (i == distanceID - 1 || i == distanceID)
            {
                int id = clamp(i - (distanceID - 1), 0, 1);
                tempDL[id] = DrawLight(input, lights[i], shadowMaps[i],
                    normalWorld, pixelToEye, albedo, metallic, roughness, tempDL[id]);

            } 
        }
             
        // ex) 5.0 <= x < 10.0 - 5.0 / 10.0 - 5.0
        float rt = distance_worldToEye - distances[distanceID] / 
                        distances[distanceID + 1] - distances[distanceID];
        rt = clamp(rt, 0.0, 1.0); 
        directLighting = tempDL[0] * (1.0 - rt) + tempDL[1] * rt;
    }
       
    output.pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    //output.pixelColor *= 1.2;
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
    
    output.indexColor = indexColor;

    
    
    return output;
}
