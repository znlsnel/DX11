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
vec3 RotateAboutZ(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) - v.y * sin(theta),
                v.x * sin(theta) + v.y * cos(theta), v.z);
}

vec3 RotateAboutY(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) + v.z * sin(theta), v.y,
                -v.x * sin(theta) + v.z * cos(theta));
}

vec3 RotateAboutX(const vec3 &v, const float &theta) {
    return vec3(v.x, v.y * cos(theta) - v.z * sin(theta),
                v.y * sin(theta) + v.z * cos(theta));
}

// 쉐이더에서 공통적으로 사용하는 상수들
// 예) 모든 버텍스들을 같은 비율로 스케일
struct Constants {
    Transformation transformation;
    Light light;
    Material material;
} constants;

// 조명효과 구현
// Lighting은 Vertex shader에서 vertex 단위로 계산할 수도 있고
// Pixel shader에서 pixel 단위로 계산할 수도 있습니다.
// 여기서는 Pixel shader에서 pixel 단위로 계산하겠습니다.

vec3 BlinnPhong(vec3 lightStrength, vec3 lightVec, vec3 normal, vec3 toEye,
                Material mat) {

    // Halfway vector 계산
    // vec3 halfway = ...

    // Halyway vector를 이용해서 specular albedo 계산
    // vec3 specular = ...

    // ambient, diffuse, specular 합쳐서 계산
    return mat.ambient;
}

vec3 ComputeDirectionalLight(Light L, Material mat, vec3 normal, vec3 toEye) {

    // 계산에 사용하는 lightVector는 directional light가 향하는 방향의 반대
    vec3 lightVec = -L.direction;

    // float ndotl = glm::max(..., 0.0f);

    vec3 lightStrength = vec3(1.0f);

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// 버텍스 쉐이더 (Vertex shader)
struct VSInput {
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
    vsOutput.position =
        RotateAboutX(
            RotateAboutY(vsInput.position * constants.transformation.scale,
                         constants.transformation.rotationY),
            constants.transformation.rotationX) +
        constants.transformation.translation;

    // 주의: 노멀 벡터도 물체와 같이 회전시켜줘야 합니다.
    // 더 편하게 구현할 방법은 없을까요?
    // 노멀 벡터가 제대로 회전됐는 지는 챕터7에서 직접 그려서 확인해보겠습니다.
    vsOutput.normal = RotateAboutX(
        RotateAboutY(vsInput.normal, constants.transformation.rotationY),
        constants.transformation.rotationX);

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

    vec3 eye = vec3(0.0f, 0.0f, -1.0f);
    vec3 toEye = glm::normalize(eye - psInput.position);
    vec3 color = ComputeDirectionalLight(constants.light, constants.material,
                                         psInput.normal, toEye);

    return vec4(color, 1.0f);
}

// 여기까지 쉐이더 정의
} // namespace hlab
