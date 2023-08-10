#include "Rasterization.h"

#include <algorithm> // std::min(), ...

namespace hlab {

using namespace glm;
using namespace std;

Rasterization::Rasterization(const int &width, const int &height)
    : width(width), height(height) {

    // 원을 그려봅시다.
    const auto radius = 0.5f;                   // 원의 반지름
    const auto center = vec3(0.0f, 0.0f, 1.0f); //원의 중심
    const size_t numTriangles = 5; // 삼각형 몇 개로 그릴지

    this->vertices.reserve(numTriangles + 1); // 중심 버텍스 추가
    this->colors.reserve(this->vertices.size());
    this->indices.reserve(numTriangles * 3);

    // 중심 버텍스의 위치와 색
    this->vertices.push_back(center);
    this->colors.push_back(vec3(1.0f, 0.0f, 0.0f));

    // 라디안(Radian) 2*PI는 360도를 의미합니다.
    // kTwoPi 이름은 구글 스타일 사용
    // https://google.github.io/styleguide/cppguide.html#Constant_Names
    const auto kTwoPi = 2.0f * 3.141592f;
    const auto deltaTheta = kTwoPi / float(numTriangles);

	// 여기서부터 this->vertices, colors, indices 결정

	
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

    // 어떤 3차원 벡터 a = (ax, ay, az)와 b = (bx, by, bz)가 있을 때,
    // a x b = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx) 이다.
    // 앞에서는 glm::cross()로 간단히 계산
    // 여기서는 3차원 정점들을 2차원 평면으로 투영했기 때문에
    // az와 bz가 둘 다 0이라고 놓고 계산하면
    // a x b = (0, 0, ax*by - ay*bx)
    // 여기서 ax*by - ay*bx 반환

    const vec2 a = v1 - v0;
    const vec2 b = point - v0;
    return a.x * b.y - a.y * b.x;
}

void Rasterization::DrawIndexedTriangle(const size_t &startIndex,
                                        vector<vec4> &pixels) {

    const size_t i0 = this->indices[startIndex];
    const size_t i1 = this->indices[startIndex + 1];
    const size_t i2 = this->indices[startIndex + 2];

    const auto v0 = ProjectWorldToRaster(this->vertices[i0]);
    const auto v1 = ProjectWorldToRaster(this->vertices[i1]);
    const auto v2 = ProjectWorldToRaster(this->vertices[i2]);

    const auto &c0 = this->colors[i0];
    const auto &c1 = this->colors[i1];
    const auto &c2 = this->colors[i2];

    const auto xMin = size_t(glm::clamp(
        glm::floor(std::min({v0.x, v1.x, v2.x})), 0.0f, float(width - 1)));
    const auto yMin = size_t(glm::clamp(
        glm::floor(std::min({v0.y, v1.y, v2.y})), 0.0f, float(height - 1)));
    const auto xMax = size_t(glm::clamp(glm::ceil(std::max({v0.x, v1.x, v2.x})),
                                        0.0f, float(width - 1)));
    const auto yMax = size_t(glm::clamp(glm::ceil(std::max({v0.y, v1.y, v2.y})),
                                        0.0f, float(height - 1)));

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

                pixels[i + width * j] = vec4(color, 1.0f);
            }
        }
    }
}

void Rasterization::Render(vector<vec4> &pixels) {
    // 삼각형 여러개 그리기
	// 한 삼각형이 vertex가 3개이기 때문에 i += 3
    for (size_t i = 0; i < this->indices.size(); i += 3) {
        DrawIndexedTriangle(i, pixels);
    }
}

void Rasterization::Update() {
    // 애니메이션 구현
}
} // namespace hlab