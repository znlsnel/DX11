#pragma once

#include "Mesh.h" // struct Material, Light

// 현대적인 그래픽스 파이프라인은 프로그래머가 직접 구현해야 하는 부분들은
// 쉐이더로 구현합니다. 그 외의 부분들은 감춰져 있습니다.

// 쉐이더 "프로그램"들을 정의하는 부분
// DirectX나 OpenGL 같은 API 에서는 별도의 파일로 분리합니다.
// (예: VertexShader.hlsl)
// 실습을 쉽게 하기 위해서 여기에 함수로 정의했습니다.
// 쉐이더와 관련된 구조체와 쉐이더 구현은 모두 자유롭게 바꿀 수 있습니다.
// 여러 가지의 쉐이더 프로그램을 만들 수도 있습니다.

namespace hlab {

using namespace glm;

// 회전은 뒤에서 행렬 배운 다음에 Rotation Matrix로 연결됩니다.
// https://en.wikipedia.org/wiki/Rotation_matrix
// vec3 RotateAboutZ(const vec3 &v, const float &theta) {
//    return vec3(v.x * cos(theta) - v.y * sin(theta),
//                   v.x * sin(theta) + v.y * cos(theta), v.z);
//}
//
// vec3 RotateAboutY(const vec3 &v, const float &theta) {
//    return vec3(v.x * cos(theta) + v.z * sin(theta), v.y,
//                   -v.x * sin(theta) + v.z * cos(theta));
//}
//
// vec3 RotateAboutX(const vec3 &v, const float &theta) {
//    return vec3(v.x, v.y * cos(theta) - v.z * sin(theta),
//                   v.y * sin(theta) + v.z * cos(theta));
//}

// 쉐이더에서 공통적으로 사용하는 상수들
// 예) 모든 버텍스들을 같은 비율로 스케일
struct Constants {

    // GPU로 Transformation 대신에 matrix 하나만 보냅니다.
    // Transformation transformation;
    mat4 modelMatrix;  // worldMatrix라고 부르기도 합니다.
    mat4 invTranspose; // Normal에 적용합니다.
    // 그 외에 viewMatrix, projectionMatrix 등도 사용합니다.

    Material material;
    Light light;
    int lightType = 0;
} constants;

// 조명효과 구현
// Lighting은 Vertex shader에서 vertex 단위로 계산할 수도 있고
// Pixel shader에서 pixel 단위로 계산할 수도 있습니다.
// 여기서는 Pixel shader에서 pixel 단위로 계산하겠습니다.

vec3 BlinnPhong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec3 toEye,
                Material mat) {

    vec3 halfway = glm::normalize(toEye + lightVec);
    vec3 specular =
        mat.specular * pow((glm::dot(halfway, normal), 0.0f), mat.shininess);

    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}

vec3 ComputeDirectionalLight(Light L, Material mat, vec3 normal, vec3 toEye) {
    vec3 lightVec = -L.direction;

    float ndotl = glm::max(glm::dot(lightVec, normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// 쉐이더에서 많이 사용되는 함수입니다. HLSL에는 내장되어 있습니다.
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-saturate
float Saturate(float x) { return glm::max(0.0f, glm::min(1.0f, x)); }

float CalcAttenuation(float d, float falloffStart, float falloffEnd) {
    // Linear falloff
    return Saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

vec3 ComputePointLight(Light L, Material mat, vec3 pos, vec3 normal,
                       vec3 toEye) {
    vec3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
        return vec3(0.0f);

    lightVec /= d;

    float ndotl = glm::max(dot(lightVec, normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;

    float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

vec3 ComputeSpotLight(Light L, Material mat, vec3 pos, vec3 normal,
                      vec3 toEye) {
    vec3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
        return vec3(0.0f);

    lightVec /= d;

    float ndotl = glm::max(dot(lightVec, normal), 0.0f);
    vec3 lightStrength = L.strength * ndotl;

    float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    lightStrength *= att;

    float spotFactor =
        glm::pow(glm::max(dot(-lightVec, L.direction), 0.0f), L.spotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// 버텍스 쉐이더 (Vertex shader)
struct VSInput {
    // CPU에서 GPU로 보내는 데이터는 적을 수록 좋기 때문에
    // vec4 대신에 vec3 사용
    vec3 position;
    vec3 normal;
    vec3 color;
    // vec2 uv;
};

struct VSOutput {
    vec3 position;
    vec3 normal;
    // vec3 color;
    //  vec2 uv;
};

VSOutput MyVertexShader(const VSInput vsInput) {
    VSOutput vsOutput;

    // 여기서 여러가지 변환 가능
    // vsOutput.position =
    //    RotateAboutX(
    //        RotateAboutY(vsInput.position * constants.transformation.scale,
    //                     constants.transformation.rotationY),
    //        constants.transformation.rotationX) +
    //    constants.transformation.translation;

    // 마지막에 1.0f 추가
    vec4 point =
        vec4(vsInput.position.x, vsInput.position.y, vsInput.position.z, 1.0f);

    // point = ...; // 주의: column-major

    vsOutput.position = vec3(point.x, point.y, point.z);

    // 주의: 노멀 벡터도 물체와 같이 회전시켜줘야 합니다.
    // 더 편하게 구현할 방법은 없을까요?
    // vsOutput.normal = RotateAboutX(
    //    RotateAboutX(
    //        RotateAboutY(vsInput.normal, constants.transformation.rotationY),
    //        constants.transformation.rotationX),
    //    constants.transformation.rotationX);

    // 마지막에 0.0f 추가
    vec4 normal =
        vec4(vsInput.normal.x, vsInput.normal.y, vsInput.normal.z, 0.0f);
    // Unon-uniform transformation인 경우에는 보정 필요
    // normal = ...; // 주의: column-major

    vsOutput.normal = vec3(normal.x, normal.y, normal.z);

    return vsOutput;

    return vsOutput;
}

// 픽셀 쉐이더 (Pixel shader)
struct PSInput {
    vec3 position;
    vec3 normal;
    // vec3 color;
    // vec2 uv;
};

vec4 MyPixelShader(const PSInput psInput) {

    vec3 eye = vec3(0.0f, 0.0f, -1.0f); // -distEyeToScreen
    vec3 toEye = glm::normalize(eye - psInput.position);

    vec3 color;

    if (constants.lightType == 0) {
        color = ComputeDirectionalLight(constants.light, constants.material,
                                        psInput.normal, toEye);
    } else if (constants.lightType == 1) {
        color = ComputePointLight(constants.light, constants.material,
                                  psInput.position, psInput.normal, toEye);
    } else {
        color = ComputeSpotLight(constants.light, constants.material,
                                 psInput.position, psInput.normal, toEye);
    }

    return vec4(color.x, color.y, color.z, 1.0f);
}

// 여기까지 쉐이더 정의
} // namespace hlab
