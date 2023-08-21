#pragma once

#include <directxtk/SimpleMath.h>

#define MAX_LIGHTS 3

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

// DirectX-Graphics-Samples/MiniEngine�� ���� �����̸� ����
// __declspec(align(256)) : DX12������ 256 align (����)

// �ַ� Vertex/Geometry ���̴����� ���
__declspec(align(256)) struct MeshConstants {
    Matrix world;
    Matrix worldIT;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    Vector2 dummy;
};

// �ַ� Pixel ���̴����� ���
__declspec(align(256)) struct MaterialConstants {

    Vector3 albedoFactor = Vector3(1.0f); // 12
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
    float expose = 1.0f;
    float gamma = 1.0f;
    Vector3 dummy;

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

// ����
struct Light {
    Vector3 radiance = Vector3(0.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 10.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 100.0f;
};

// register(b1) ���
__declspec(align(256)) struct GlobalConstants {
    Matrix view;
    Matrix proj;
    Matrix viewProj;
    Vector3 eyeWorld;
    float strengthIBL = 1.0f;
    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, �׿�: ������
    float envLodBias = 0.0f; // ȯ��� LodBias
    float lodBias = 2.0f;    // �ٸ� ��ü�� LodBias
    float dummy2;

    Light lights[MAX_LIGHTS];
};

} // namespace hlab
