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
    Matrix view;
    Matrix projection;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    Vector2 dummy;
};

struct BasicPixelConstData {
    Vector3 eyeWorld;         // 12
    float mipmapLevel = 0.0f; // 4

    Material material;        // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS

    // 여러 옵션들에 uint 플래그 하나만 사용할 수도 있습니다.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;         // Ambient Occlusion
    int invertNormalMapY = 0; // 16
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float expose = 1.0f; // 16
    float gamma = 1.0f;
    Vector3 dummy; // 16
};

struct NormalVertexConstantData {
    float scale = 0.1f;
    float dummy[3];
};

} // namespace hlab
