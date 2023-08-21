#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#define MAX_LIGHTS 3

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// Α¶Έν
struct Light {
    Vector3 radiance = Vector3(0.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 10.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 100.0f;

    // Light type bitmasking 
    // 0x00: inactive
    // 0x01: directional, 0x02: point, 0x03: spot,
    // shadowmap on/off: 0x10
    uint32_t type = 0x00;
    Vector3 dummy;
};
} // namespace hlab
