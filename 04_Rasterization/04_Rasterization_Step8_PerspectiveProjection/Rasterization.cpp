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
    float rotationX = 0.0f;
    float rotationZ = 0.0f;
} constants;

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
    vsOutput.position =
        RotateAboutX(vsInput.position * constants.scale, constants.rotationX) +
        constants.translation;
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

    // https://www.shadertoy.com/view/lt2XWK
    // mod는 glm::mod 사용, float 주의 2. -> 2.0f
    float Size = 5.0f;
    vec2 Pos = glm::mod(floor(psInput.uv * Size), 2.0f);
    return glm::mod(Pos.x + Pos.y, 2.0f)
               ? vec4(psInput.color, 1.0f)
               : vec4(1.0f, 0.0f, 0.0f, 1.0f) * psInput.uv.y +
                     vec4(0.0f, 0.0f, 1.0f, 1.0f) * (1.0f - psInput.uv.y);
}

// 여기까지 쉐이더 정의

// 여기부터 다시 Rasterization

Rasterization::Rasterization(const int &width, const int &height)
    : width(width), height(height) {

    // 거리가 다른 사각형 두 개
    // 약간 회전시켰기 때문에 정투영으로 보면
    // 가로가 약간 길어보입니다.

    auto square1 = make_shared<Mesh>();
    square1->InitSquare(1.0f, 1.0f, vec3(1.0f));
    square1->rotationX = -3.141592f * 30 / 180;
    square1->scale = vec3(1.0f);
    square1->translation = vec3(-0.6, 0.0f, 0.1f);
    square1->CopyToBuffer();

    meshes.push_back(square1);

    auto square2 = make_shared<Mesh>();
    square2->InitSquare(1.0f, 1.0f, vec3(1.0f));
    square2->rotationX = -3.141592f * 30 / 180;
    square2->scale = vec3(1.0f);
    square2->translation = vec3(0.6, 0.0f, 1.0f);
    square2->CopyToBuffer();

    meshes.push_back(square2);

    // 깊이 버퍼 초기화
    depthBuffer.resize(width * height, 0.0f);
}

// 3차원 좌표를 2차원 좌표로 변환
vec2 Rasterization::ProjectWorldToRaster(vec3 pointWorld) {

    // 월드 좌표계의 원점이 우리가 보는 화면의 중심이라고 가정

    // 정투영(Orthographic projection)
    vec2 pointProj = vec2(pointWorld.x, pointWorld.y);

    // 원근투영(Perspective projection)
    if (this->usePerspectiveProjection) {
        // ... 
    }

    // NDC로 변환[-1, 1] x[-1, 1]
    // NDC(Normalized Device Coordinates)
    // NDC는 모니터의 실제 해상도와 상관 없이 정사각형이라는 점 주의
    // 그림: http://www.directxtutorial.com/Lesson.aspx?lessonid=111-4-1
    // 여기서는 width가 height보다 긴 경우만 고려

    const float aspect = float(width) / height;
    const vec2 pointNDC = vec2(pointProj.x / aspect, pointProj.y);

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

    // 삼각형 전체 넓이의 두 배, 음수일 수도 있음
    const float area = EdgeFunction(v0, v1, v2);

    // 뒷면일 경우 그리지 않음
    if (this->cullBackface && area < 0.0f)
        return;

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

            // 위에서 계산한 삼각형 전체 넓이 area를 재사용
            // area가 음수라면 alpha0, alpha1, alpha2 모두 음수여야
            // 삼각형 안에 포함되는 픽셀로 판단할 수 있습니다.
            float w0 = EdgeFunction(v1, v2, point) / area;
            float w1 = EdgeFunction(v2, v0, point) / area;
            float w2 = EdgeFunction(v0, v1, point) / area;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {

                // Perspective-Correct Interpolation
                // 논문
                // https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
                // 해설글
                // https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
                // OpenGL 구현
                // https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                const float z0 = this->vertexBuffer[i0].z + distEyeToScreen;
                const float z1 = this->vertexBuffer[i1].z + distEyeToScreen;
                const float z2 = this->vertexBuffer[i2].z + distEyeToScreen;

                if (this->usePerspectiveProjection &&
                    this->usePerspectiveCorrectInterpolation) {

                       // w0, w1, w2를 z0, z1, z2를 이용해서 보정

                }

                // 이하 동일
                const float depth = w0 * z0 + w1 * z1 + w2 * z2;
                const vec3 color = w0 * c0 + w1 * c1 + w2 * c2;
                const vec2 uv = w0 * uv0 + w1 * uv1 + w2 * uv2;

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
    // 깊이 버퍼의 초기값은 렌더링할 가장 먼 거리를 설정해준다는 의미도
    // 있습니다. 즉, 그 거리보다 더 멀리있는 픽셀은 무시하게 됩니다.
    // DirectX에서는 내부적으로 렌더링할 공간을 스케일해서 가장 먼
    // 거리를 1.0f으로 만들기 때문에 보통 1.0으로 초기화하지만,
    // 여기서는 편의상 1.0보다 큰 값(예: 10.0f)을 사용하겠습니다.
    fill(this->depthBuffer.begin(), this->depthBuffer.end(), 10.0f);

    for (const auto &mesh : this->meshes) {

        constants.rotationX = mesh->rotationX;
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
}
} // namespace hlab