#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

// 참고자료
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

// 메쉬 재질 텍스춰들 t0 부터 시작
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0

cbuffer MaterialConstants : register(b0)
{
    float3 albedoFactor; // baseColor
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float dummy;
};

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
    //return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

float3 GetNormal(PixelShaderInput input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb;
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
    float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    float2 specularBRDF = brdfTex.SampleLevel(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float3 specularIrradiance = specularIBLTex.SampleLevel(linearWrapSampler, reflect(-pixelToEye, normalWorld),
                                                            2 + roughness * 5.0f).rgb;
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
float NdfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;

    return alphaSq / (3.141592 * denom * denom);
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

float3 LightRadiance(Light light, float3 posWorld, float3 normalWorld, Texture2D shadowMap)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
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
        float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
        lightScreen.xyz /= lightScreen.w;
        
        // 2. 카메라(광원)에서 볼 때의 텍스춰 좌표 계산
        float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
        lightTexcoord += 1.0;
        lightTexcoord *= 0.5;
        
        // 3. 쉐도우맵에서 값 가져오기
        float depth = shadowMap.Sample(shadowPointSampler, lightTexcoord).r;
        
        // 4. 가려져 있다면 그림자로 표시
        if (depth + 0.001 < lightScreen.z)
            shadowFactor = 0.0;
        
        
        
        uint width, height, numMips;
        shadowMap.GetDimensions(0, width, height, numMips);
        
        float dx = 5.0 / (float) width;
        float percentLit = 0.0;
        const float2 offsets[64] =
        {
            float2(0.0, 0.0),
float2(-0.12499844227275288, 0.000624042775189866), float2(0.1297518688031755, -0.12006020382326336),
float2(-0.017851253586055427, 0.21576916541852392), float2(-0.1530983013115895, -0.19763833164521946),
float2(0.27547541035593626, 0.0473106572479027), float2(-0.257522587854559, 0.16562643733622642),
float2(0.0842605283808073, -0.3198048832600703), float2(0.1645196099088727, 0.3129429627830483),
float2(-0.3528833088400373, -0.12687935349026194), float2(0.36462214742013344, -0.1526456341030772),
float2(-0.17384046457324884, 0.37637015407303087), float2(-0.1316547617859344, -0.4125130588224921),
float2(0.3910687393754993, 0.2240317858770442), float2(-0.45629121277761536, 0.10270505898899496),
float2(0.27645268679640483, -0.3974278701387824), float2(0.06673001731984558, 0.49552709793561556),
float2(-0.39574431915605623, -0.33016879600548193), float2(0.5297612167716342, -0.024557141621887494),
float2(-0.3842909284448636, 0.3862583103507092), float2(0.0230336562454131, -0.5585422550532486),
float2(0.36920334463249477, 0.43796562686149154), float2(-0.5814490172413539, -0.07527974727019048),
float2(0.4903718680780365, -0.3448339179919178), float2(-0.13142003698572613, 0.5981043168868373),
float2(-0.31344141845114937, -0.540721256470773), float2(0.608184438565748, 0.19068741092811003),
float2(-0.5882602609696388, 0.27536315179038107), float2(0.25230610046544444, -0.6114259003901626),
float2(0.23098706800827415, 0.6322736546883326), float2(-0.6076303951666067, -0.31549215975943595),
float2(0.6720886334230931, -0.1807536135834609), float2(-0.37945598830371974, 0.5966683776943834),
float2(-0.1251555455510758, -0.7070792667147104), float2(0.5784815570900413, 0.44340623372555477),
float2(-0.7366710399837763, 0.0647362251696953), float2(0.50655463562529, -0.553084443034271),
float2(8.672987356252326e-05, 0.760345311340794), float2(-0.5205650355786364, -0.5681215043747359),
float2(0.7776435491294021, 0.06815798190547596), float2(-0.6273416101921778, 0.48108471615868836),
float2(0.1393236805531513, -0.7881712453757264), float2(0.4348773806743975, 0.6834703093608201),
float2(-0.7916014213464706, -0.21270211499241704), float2(0.7357897682897174, -0.38224784745000717),
float2(-0.2875567908732709, 0.7876776574352392), float2(-0.3235695699691864, -0.7836151691933712),
float2(0.7762165924462436, 0.3631291803355136), float2(-0.8263007976064866, 0.2592816844184794),
float2(0.4386452756167397, -0.7571098481588484), float2(0.18988542402304126, 0.8632459242554175),
float2(-0.7303253445407815, -0.5133224046555819), float2(0.8939004035324556, -0.11593993515830946),
float2(-0.5863762307291154, 0.6959079795748251), float2(-0.03805753378232556, -0.9177699189461416),
float2(0.653979655650389, 0.657027860897389), float2(-0.9344208130797295, -0.04310155546401203),
float2(0.7245109901504777, -0.6047386420191574), float2(-0.12683493131695708, 0.9434844461875473),
float2(-0.5484582700240663, -0.7880790100251422), float2(0.9446610338564589, 0.2124041692463835),
float2(-0.8470120123194587, 0.48548496473788055), float2(0.29904134279525085, -0.9377229203230629),
float2(0.41623562331748715, 0.9006236205438447),
        };
        
        //[unroll]
        //for (int i = 0; i < 64; i++)
        //{
        // //   int index = int(16.0 * random(posWorld.xyz, i)) % 16u;
        //    percentLit +=shadowMap.
        //    SampleCmpLevelZero(shadowCompareSampler, lightTexcoord.xy + (offsets[i] * dx), lightScreen.z - 0.001).r;
        //}
        //shadowFactor = percentLit / 64.0;


         //   int index = int(16.0 * random(posWorld.xyz, i)) % 16u;
            percentLit += shadowMap.
            SampleCmpLevelZero(shadowCompareSampler, lightTexcoord.xy, lightScreen.z - 0.001).r;
        
        shadowFactor = percentLit;
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}


float3 LightRadiance(in Light light, in float3 posWorld, in float3 normalWorld)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
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
    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

PixelShaderOutput main(PixelShaderInput input)
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    float3 albedo = useAlbedoMap ? albedoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb * albedoFactor
                                 : albedoFactor;
    
    float ao = useAOMap ? aoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).r : 1.0;
    float metallic = useMetallicMap ? metallicRoughnessTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).b * metallicFactor
                                    : metallicFactor;
    float roughness = useRoughnessMap ? metallicRoughnessTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).g * roughnessFactor
                                      : roughnessFactor;
    float3 emission = useEmissiveMap ? emissiveTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb
                                     : emissionFactor;

    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao, metallic, roughness) * strengthIBL;
    
    float3 directLighting = float3(0, 0, 0);

    // 임시로 unroll 사용
    [unroll] // warning X3550: sampler array index must be a literal expression, forcing loop to unroll
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (lights[i].type)
        {
            float3 lightVec = lights[i].position - input.posWorld;
            float lightDist = length(lightVec);
            lightVec /= lightDist;
            float3 halfway = normalize(pixelToEye + lightVec);
        
            float NdotI = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfway));
            float NdotO = max(0.0, dot(normalWorld, pixelToEye));
        
            const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
            float3 F0 = lerp(Fdielectric, albedo, metallic);
            float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
            float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
            float3 diffuseBRDF = kd * albedo;

            float D = NdfGGX(NdotH, roughness);
            float3 G = SchlickGGX(NdotI, NdotO, roughness);
            float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

            float3 radiance = 0.0f;
            
            radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMaps[i]);
            
            /*if (i == 0)
                radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMap0);
            if (i == 1)
                radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMap1);
            if (i == 2)
                radiance = LightRadiance(lights[i], input.posWorld, normalWorld, shadowMap2);*/
                
            directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
        }
    }
    
    PixelShaderOutput output;
    output.pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);

    return output;
}
