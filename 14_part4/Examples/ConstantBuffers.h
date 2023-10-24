#pragma once

#include "D3D11Utils.h"
#include <directxtk/SimpleMath.h>

// "Common.hlsli"�� �����ؾ� ��
#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// DirectX-Graphics-Samples/MiniEngine�� ���� �����̸� ����
// __declspec(align(256)) : DX12������ 256 align (����)

// �ַ� Vertex/Geometry ���̴����� ���
__declspec(align(256)) struct MeshConstants {
    Matrix world;
    Matrix worldIT;
    Matrix worldInv;

    //Vector4 indexColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    //Vector4 indexColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    float indexColor[4] = {0.0f, 0.0f, 0.0f, 0.7865f};
   //int indexColor[4] = {0, 0, 0, 255};

    int useHeightMap = 0;
    float heightScale = 0.0f;
    float windTrunk = 0.0f;
    float windLeaves = 0.0f;
};

__declspec(align(256)) struct TessellationConstants {
    Vector3 eyeWorld;
    float padding = 0.0f;
    Matrix model;
    Matrix view;
    Matrix proj;
};

// �ַ� Pixel ���̴����� ���
__declspec(align(256)) struct MaterialConstants {

    Vector3 albedoFactor = Vector3(1.0f);
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Vector3 emissionFactor = Vector3(0.0f);

    // ���� �ɼǵ鿡 uint �÷��� �ϳ��� ����� ���� �ֽ��ϴ�.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    int isSelected = 0;

    // ���� flags ����
    /* union {
        uint32_t flags;
        struct {
            // UV0 or UV1 for each texture
            uint32_t baseColorUV : 1;
            uint32_t metallicRoughnessUV : 1;
            uint32_t occlusionUV : 1;
            uint32_t emissiveUV : 1;
            uint32_t normalUV : 1;

            // Three special modes
            uint32_t twoSided : 1;
            uint32_t alphaTest : 1;
            uint32_t alphaBlend : 1;

            uint32_t _pad : 8;

            uint32_t alphaRef : 16; // half float
        };
    };*/
};

struct Light {
    Vector3 radiance = Vector3(5.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 20.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 6.0f;

    // Light type bitmasking
    // ex) LIGHT_SPOT | LIGHT_SHADOW
    uint32_t type = LIGHT_OFF;
    float radius = 0.035f; // ������

    float haloRadius = 0.0f;
    float haloStrength = 0.0f;

    Matrix viewProj; // �׸��� �������� �ʿ�
    Matrix invProj;  // �׸��� ������ ������
};

// register(b1) ���
__declspec(align(256)) struct GlobalConstants {
    Matrix view;
    Matrix proj;
    Matrix invProj; // �������������
    Matrix viewProj;
    Matrix invViewProj; // Proj -> World
    Matrix invView;
     
    Vector3 eyeWorld;
    float strengthIBL = 0.5f;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, �׿�: ������
    float envLodBias = 0.0f; // ȯ��� LodBias
    float lodBias = 0.0f;    // �ٸ� ��ü�� LodBias
    float globalTime = 0.0f;

    Light lights[MAX_LIGHTS];
};

// register(b5) ���, PostEffectsPS.hlsl
__declspec(align(256)) struct PostEffectsConstants {
    int mode = 1; // 1: Rendered image, 2: DepthOnly
    float depthScale = 1.0f;
    float fogStrength = 0.0f;
};

__declspec(align(256)) struct VolumeConsts {
    Vector3 uvwOffset = Vector3(0.0f);
    float lightAbsorption = 5.0f;
    Vector3 lightDir = Vector3(0.0f, 1.0f, 0.0f);
    float densityAbsorption = 10.0f;
    Vector3 lightColor = Vector3(1, 1, 1) * 40.0f;
    float aniso = 0.3f;
};

// bone ���� ������ ���ֱ� ���� StructuredBuffer�� ��ü
// __declspec(align(256)) struct SkinnedConsts {
//    Matrix boneTransforms[52]; // bone ����
//};

template <typename T_CONSTS> class ConstantBuffer {
  public:
    void Initialize(ComPtr<ID3D11Device> &device) {
        D3D11Utils::CreateConstBuffer(device, m_cpu, m_gpu);
    }

    void Upload(ComPtr<ID3D11DeviceContext> &context) {
        D3D11Utils::UpdateBuffer(context, m_cpu, m_gpu);
    }

  public:
    T_CONSTS &GetCpu() { return m_cpu; }
    const auto Get() { return m_gpu.Get(); }
    const auto GetAddressOf() { return m_gpu.GetAddressOf(); }

    T_CONSTS m_cpu;
    ComPtr<ID3D11Buffer> m_gpu;
};

} // namespace hlab
