#pragma once

#include <directxtk/SimpleMath.h>

#include "Light.h"
#include "Material.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

struct BasicVertexConstData {
    Matrix modelWorld;
    Matrix invTranspose;
    // Matrix view;  // �ٸ� Const�� �и�
    // Matrix projection; // �ٸ� Const�� �и�
    int useHeightMap = 0;
    float heightScale = 0.0f;
    Vector2 dummy;
};

struct EyeViewProjConstData {
    // View�� Proj�� �̸� ���� ����
    Matrix viewProj;
    Vector3 eyeWorld; // Eye�� �и� ����
    float dummy;
};

struct BasicPixelConstData {
    Material material;        // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS

    // ���� �ɼǵ鿡 uint �÷��� �ϳ��� ����� ���� �ֽ��ϴ�.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;         // Ambient Occlusion
    int invertNormalMapY = 0; // 16
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float expose = 1.0f; // 16
    float gamma = 1.0f;
    float mipmapLevel = 0.0f;
    Vector2 dummy; // 16
};

struct NormalVertexConstantData {
    float scale = 0.1f;
    float dummy[3];
};

} // namespace hlab
