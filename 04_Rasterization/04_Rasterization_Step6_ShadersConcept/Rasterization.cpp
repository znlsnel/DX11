#include "Rasterization.h"

#include <algorithm> // std::min(), ...

namespace hlab {

using namespace glm;
using namespace std;

vec3 RotateAboutZ(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) - v.y * sin(theta),
                v.x * sin(theta) + v.y * cos(theta), v.z);
}

// RotateAboutZ(.)를 약간만 변경
vec3 RotateAboutX(const vec3 &v, const float &theta) {
    return vec3(v.x, v.y * cos(theta) - v.z * sin(theta),
                v.y * sin(theta) + v.z * cos(theta));
}

// 현대적인 그래픽스 파이프라인은 프로그래머가 직접 구현해야 하는 부분들은
// 쉐이더로 구현합니다. 그 외의 부분들은 감춰져 있습니다.

// 쉐이더 "프로그램"들을 정의하는 부분
// DirectX나 OpenGL 같은 API 에서는 별도의 파일로 분리합니다.
// (예: VertexShader.hlsl)
// 실습을 쉽게 하기 위해서 여기에 함수로 정의했습니다.
// 쉐이더와 관련된 구조체와 쉐이더 구현은 모두 자유롭게 바꿀 수 있습니다.
// 여러 가지의 쉐이더 프로그램을 만들 수도 있습니다.

// 쉐이더에서 공통적으로 사용하는 상수들
// 예) 모든 버텍스들을 같은 비율로 스케일
struct Constants {
    //중요: 이것들을 하나로 합칠 순 없을까?
    vec3 scale = vec3(1.0f);
    vec3 translation = vec3(0.0f);
    float rotationZ = 0.0f;
} constants;// g_constants

// 버텍스 쉐이더 (Vertex shader)
struct VSInput {
    vec3 position;
    vec3 color;
    vec2 uv;
};

struct VSOutput {
    vec3 position;
    vec3 color;
    vec2 uv;
};

VSOutput MyVertexShader(const VSInput vsInput) {
    VSOutput vsOutput;

    // 여기서 여러가지 변환 가능
    vsOutput.position = vsInput.position;
    vsOutput.color = vsInput.color;
    vsOutput.uv = vsInput.uv;

    return vsOutput;
}

// 픽셀 쉐이더 (Pixel shader)
struct PSInput {
    vec3 color;
    vec2 uv;
};

vec4 MyPixelShader(const PSInput psInput) { 
	
    // 여기서 픽셀의 색을 결정하기 위한 여러가지 규칙 적용
    return vec4(psInput.color, 1.0f); 
}

// 여기까지 쉐이더 정의

// 여기부터 다시 Rasterization

Rasterization::Rasterization(const int &width, const int &height)
    : width(width), height(height) {

    auto square1 = make_shared<Mesh>();
    square1->InitSquare(1.0f, 1.0f, vec3(1.0f));
    square1->scale = vec3(1.0f, 1.0f, 1.0f);
    square1->CopyToBuffer();

    meshes.push_back(square1);

    // 깊이 버퍼 초기화
    depthBuffer.resize(width * height, 0.0f);
}

// 3차원 좌표를 2차원 좌표로 변환
// 이번 예제에서는 정투영(Orthographic projection) 사용
vec2 Rasterization::ProjectWorldToRaster(vec3 point) {

    // 월드 좌표계의 원점이 우리가 보는 화면의 중심이라고 가정

    // NDC로 변환[-1, 1] x[-1, 1]
    // NDC(Normalized Device Coordinates)
    // NDC는 모니터의 실제 해상도와 상관 없이 정사각형이라는 점 주의
    // 그림: http://www.directxtutorial.com/Lesson.aspx?lessonid=111-4-1
    // 여기서는 width가 height보다 긴 경우만 고려

    const float aspect = float(width) / height;
    const vec2 pointNDC = vec2(point.x / aspect, point.y);

    // 레스터 좌표의 범위 [-0.5, width - 1 + 0.5] x [-0.5, height - 1 + 0.5]
    const float xScale = 2.0f / width;
    const float yScale = 2.0f / height;

    // NDC -> 레스터 화면 좌표계
    // 주의: y좌표 상하반전
    return vec2((pointNDC.x + 1.0f) / xScale - 0.5f,
                (1.0f - pointNDC.y) / yScale - 0.5f);
}

float Rasterization::EdgeFunction(const vec2 &v0, const vec2 &v1,
                                  const vec2 &point) {
    const vec2 a = v1 - v0;
    const vec2 b = point - v0;
    return a.x * b.y - a.y * b.x;
}

void Rasterization::DrawIndexedTriangle(const size_t &startIndex,
                                        vector<vec4> &pixels) {

    const size_t i0 = this->indexBuffer[startIndex];
    const size_t i1 = this->indexBuffer[startIndex + 1];
    const size_t i2 = this->indexBuffer[startIndex + 2];

    const auto v0 = ProjectWorldToRaster(this->vertexBuffer[i0]);
    const auto v1 = ProjectWorldToRaster(this->vertexBuffer[i1]);
    const auto v2 = ProjectWorldToRaster(this->vertexBuffer[i2]);

    const auto &c0 = this->colorBuffer[i0];
    const auto &c1 = this->colorBuffer[i1];
    const auto &c2 = this->colorBuffer[i2];

    const auto &uv0 = this->uvBuffer[i0];
    const auto &uv1 = this->uvBuffer[i1];
    const auto &uv2 = this->uvBuffer[i2];

    const auto xMin = size_t(glm::clamp(
        glm::floor(std::min({v0.x, v1.x, v2.x})), 0.0f, float(width - 1)));
    const auto yMin = size_t(glm::clamp(
        glm::floor(std::min({v0.y, v1.y, v2.y})), 0.0f, float(height - 1)));
    const auto xMax = size_t(glm::clamp(glm::ceil(std::max({v0.x, v1.x, v2.x})),
                                        0.0f, float(width - 1)));
    const auto yMax = size_t(glm::clamp(glm::ceil(std::max({v0.y, v1.y, v2.y})),
                                        0.0f, float(height - 1)));

    // GPU 안에서는 멀티쓰레딩으로 여러 픽셀들을 한꺼번에 처리합니다.
    // 엄밀히 얘기하면 픽셀이 아니라 될 수 있는 "후보"들이기 때문에
    // Fragment라는 다른 용어를 사용하기도 합니다.
    // OpenGL, Vulkan에서는 Fragment Shader, DX에서는 Pixel Shader
    for (size_t j = yMin; j <= yMax; j++) {
        for (size_t i = xMin; i <= xMax; i++) {

            const vec2 point = vec2(float(i), float(j));
            const float alpha0 = EdgeFunction(v1, v2, point);
            const float alpha1 = EdgeFunction(v2, v0, point);
            const float alpha2 = EdgeFunction(v0, v1, point);

            if (alpha0 >= 0.0f && alpha1 >= 0.0f && alpha2 >= 0.0f) {

                const float area = alpha0 + alpha1 + alpha2;
                const vec3 color =
                    (alpha0 * c0 + alpha1 * c1 + alpha2 * c2) / area;

                // UV 좌표(텍스춰 좌표)도 Bary-centric coordinates를 이용해서
                // interpolation
                const vec2 uv =
                    (alpha0 * uv0 + alpha1 * uv1 + alpha2 * uv2) / area;

                // 정투영(orthographic projection)에서만 정확합니다.
                // 뒤에서 Perspective Correct Interpolation으로 보정
                const float depth = (alpha0 * this->vertexBuffer[i0].z +
                                     alpha1 * this->vertexBuffer[i1].z +
                                     alpha2 * this->vertexBuffer[i2].z) /
                                    area;

                if (depth < depthBuffer[i + width * j]) {
                    depthBuffer[i + width * j] = depth;

                    PSInput psInput;
                    psInput.color = color;
                    psInput.uv = uv;

                    pixels[i + width * j] = MyPixelShader(psInput);
                }
            }
        }
    }
}

void Rasterization::Render(vector<vec4> &pixels) {

    // 깊이 버퍼 초기화
    this->depthBuffer.resize(pixels.size());
    fill(this->depthBuffer.begin(), this->depthBuffer.end(),
         1.0f); // 큰 값으로 초기화

    for (const auto &mesh : this->meshes) {

        constants.rotationZ = mesh->rotationZ;
        constants.scale = mesh->scale;
        constants.translation = mesh->translation;

        this->vertexBuffer.resize(mesh->vertexBuffer.size());
        this->colorBuffer.resize(mesh->colorBuffer.size());
        this->uvBuffer.resize(mesh->uvBuffer.size());

        // GPU 안에서는 멀티쓰레딩으로 여러 버텍스를 한꺼번에 처리합니다.
        for (size_t i = 0; i < mesh->vertexBuffer.size(); i++) {

            VSInput vsInput;
            vsInput.position = mesh->vertexBuffer[i];
            vsInput.color = mesh->colorBuffer[i];
            vsInput.uv = mesh->uvBuffer[i];

            auto vsOutput = MyVertexShader(vsInput);

            this->vertexBuffer[i] = vsOutput.position;
            this->colorBuffer[i] = vsOutput.color;
            this->uvBuffer[i] = vsOutput.uv;
        }

        this->indexBuffer = mesh->indexBuffer;

        for (size_t i = 0; i < this->indexBuffer.size(); i += 3) {
            DrawIndexedTriangle(i, pixels);
        }
    }
}

void Rasterization::Update() {
    // 애니메이션 구현

    for (auto &mesh : this->meshes) {
        mesh->rotationZ += 0.001f;
    }
}
} // namespace hlab