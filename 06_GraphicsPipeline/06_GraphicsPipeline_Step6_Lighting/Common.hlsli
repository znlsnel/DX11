// 쉐이더에서 include할 내용들은 .hlsli 파일에 작성
// Properties -> Item Type: Does not participate in build으로 설정

// BlinnPhong 구현의 전체 구조는 Luna DX12 교재와 비슷하지만 
// 세부 구현은 이해하기 편하도록 대학 강의 스타일로 단순화하였습니다.

/* 참고: C++ SimpleMath -> HLSL */
// Matrix -> matrix 또는 float4x4
// Vector3 -> float3
// float3 a = normalize(b);
// float a = dot(v1, v2);
// Satuarate() -> saturate() 사용
// float l = length(v);
// struct A{ float a = 1.0f; }; <- 구조체 안에서 초기화 불가
// Vector3(0.0f) -> float3(0.0, 0.0, 0.0) // 실수 뒤에 f 불필요
// Vector4::Transform(v, M) -> mul(v, M)

#define MAX_LIGHTS 3 // 쉐이더에서도 #define 사용 가능
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

// 재질
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; // 16 bytes 맞춰주기 위해 추가
    float3 specular;
    float dummy2;
};

// 조명
struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, Material mat)
{
    // TODO:
    return mat.ambient;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                                float3 toEye)
{
    // TODO:
    return float3(1.0, 1.0, 1.0);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                          float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        // TODO:
        return float3(1.0, 1.0, 1.0);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        // TODO:
        return float3(1.0, 1.0, 1.0);
    }
    
    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}

struct VertexShaderInput
{
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD0; // <- 다음 예제에서 사용
    
    // float3 color : COLOR0; <- 불필요 (쉐이딩)
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    
    // float3 color : COLOR; <- 불필요 (쉐이딩)
};
