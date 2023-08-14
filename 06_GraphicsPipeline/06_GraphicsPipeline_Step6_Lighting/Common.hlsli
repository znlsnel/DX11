// ���̴����� include�� ������� .hlsli ���Ͽ� �ۼ�
// Properties -> Item Type: Does not participate in build���� ����

// BlinnPhong ������ ��ü ������ Luna DX12 ����� ��������� 
// ���� ������ �����ϱ� ���ϵ��� ���� ���� ��Ÿ�Ϸ� �ܼ�ȭ�Ͽ����ϴ�.

/* ����: C++ SimpleMath -> HLSL */
// Matrix -> matrix �Ǵ� float4x4
// Vector3 -> float3
// float3 a = normalize(b);
// float a = dot(v1, v2);
// Satuarate() -> saturate() ���
// float l = length(v);
// struct A{ float a = 1.0f; }; <- ����ü �ȿ��� �ʱ�ȭ �Ұ�
// Vector3(0.0f) -> float3(0.0, 0.0, 0.0) // �Ǽ� �ڿ� f ���ʿ�
// Vector4::Transform(v, M) -> mul(v, M)

#define MAX_LIGHTS 3 // ���̴������� #define ��� ����
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

// ����
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; // 16 bytes �����ֱ� ���� �߰�
    float3 specular;
    float dummy2;
};

// ����
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
    float3 h = normalize(toEye + lightVec);
    float hdotl = max(dot(h, normal), 0.0);
    float3 specular = mat.specular * pow(hdotl, mat.shininess);
    
    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                                float3 toEye)
{
    // TODO:
    float3 lightVec = -L.direction;
    float lightStrength = max(dot(lightVec, normal), 0.0) * L.strength;
    
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);

}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                          float3 toEye)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    // �ʹ� �ָ� ������ ������� ����
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;
        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        float3 lightStrength = max(dot(lightVec, normal), 0.0) * L.strength * att;
        
        // TODO:
        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    // �ʹ� �ָ� ������ ������� ����
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        // TODO:
        lightVec /= d;
        float3 lightStrength = max(dot(lightVec, normal), 0.0);
        
        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0), L.spotPower);
        lightStrength *=  L.strength * att * spotFactor;
        
        
        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
    
    // if�� else�� ���� ��� ��� �߻�
    // warning X4000: use of potentially uninitialized variable
}

struct VertexShaderInput
{
    float3 posModel : POSITION; //�� ��ǥ���� ��ġ position
    float3 normalModel : NORMAL; // �� ��ǥ���� normal    
    float2 texcoord : TEXCOORD0; // <- ���� �������� ���
    
    // float3 color : COLOR0; <- ���ʿ� (���̵�)
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION; // World position (���� ��꿡 ���)
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    
    // float3 color : COLOR; <- ���ʿ� (���̵�)
};
