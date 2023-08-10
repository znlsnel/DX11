#include "Rasterization.h"

#include <algorithm> // std::min(), ...

namespace hlab {

using namespace glm;
using namespace std;

Rasterization::Rasterization(const int &width, const int &height)
    : width(width), height(height) {

    // Mesh 클래스에서 원으로 초기화
    
    circle1.InitCircle(0.3f, 30, vec3(1.0f, 0.0f, 0.0f)); // red
    circle2.InitCircle(0.5f, 30, vec3(0.0f, 0.0f, 1.0f)); // blue
    circle3.InitCircle(0.5f, 30, vec3(1.0f, 1.0f, 0.0f)); // yellow

    center1 = vec3(0.0f, 0.0f, 0.1f); // 가장 앞
	center2 = vec3(-0.3, 0.0f, 0.3f);
    center3 = vec3(0.3, 0.0f, 0.7f); // 화면에서 더 멀도록 배치

    // 버퍼로 복사 (GPU 메모리에 사본을 만든다고 생각합시다.)
    circle1.vertexBuffer = circle1.vertices;
    circle1.indexBuffer = circle1.indices;
    circle1.colorBuffer = circle1.colors;

    circle2.vertexBuffer = circle2.vertices;
    circle2.indexBuffer = circle2.indices;
    circle2.colorBuffer = circle2.colors;

    circle3.vertexBuffer = circle3.vertices;
    circle3.indexBuffer = circle3.indices;
    circle3.colorBuffer = circle3.colors;

    // sun.vertices, indices, colors는 GPU에 사본이 있기 때문에
    // 여기에서 삭제 가능
    circle1.vertices.clear();
    circle1.indices.clear();
    circle1.colors.clear();

    circle2.vertices.clear();
    circle2.indices.clear();
    circle2.colors.clear();

    circle3.vertices.clear();
    circle3.indices.clear();
    circle3.colors.clear();
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

    const size_t i0 = this->indexBuffer[startIndex];
    const size_t i1 = this->indexBuffer[startIndex + 1];
    const size_t i2 = this->indexBuffer[startIndex + 2];

    const auto v0 = ProjectWorldToRaster(this->vertexBuffer[i0]);
    const auto v1 = ProjectWorldToRaster(this->vertexBuffer[i1]);
    const auto v2 = ProjectWorldToRaster(this->vertexBuffer[i2]);

    const auto &c0 = this->colorBuffer[i0];
    const auto &c1 = this->colorBuffer[i1];
    const auto &c2 = this->colorBuffer[i2];

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

				// 정투영(orthographic projection)에서만 정확합니다.
				// 뒤에서 Perspective Correct Interpolation으로 보정
                //TODO: Bary-centric coordinates를 이용해서 z 좌표 찾기
                //const float depth = ...;

                //TODO: 조건 추가
				if (true) {
                    //TODO: 깊이 버퍼 업데이트
                    pixels[i + width * j] = vec4(color, 1.0f);
                }
            }
        }
    }
}

vec3 RotateAboutZ(const vec3 &v, const float &theta) {
    return vec3(v.x * cos(theta) - v.y * sin(theta),
                v.x * sin(theta) + v.y * cos(theta), v.z);
}

void Rasterization::Render(vector<vec4> &pixels) {

	// 깊이 버퍼 초기화
	this->depthBuffer.resize(pixels.size());
    
    //TODO: 깊이 버퍼의 값도 초기화해줘야 합니다.
	// std::fill() 사용

    // 뒷쪽의 원을 나중에 그리기
    this->vertexBuffer.resize(circle1.vertexBuffer.size());
    for (size_t i = 0; i < circle1.vertexBuffer.size(); i++) {
        this->vertexBuffer[i] = circle1.vertexBuffer[i] + center1;
    }

    this->indexBuffer = circle1.indexBuffer;
    this->colorBuffer = circle1.colorBuffer;

    // 현재 버퍼로 삼각형 하나씩 그리기 (아래 for루프는 여러번 사용됨)
    for (size_t i = 0; i < this->indexBuffer.size(); i += 3) {
        DrawIndexedTriangle(i, pixels);
    }

    this->vertexBuffer.resize(circle2.vertexBuffer.size());
    for (size_t i = 0; i < circle2.vertexBuffer.size(); i++) {
        this->vertexBuffer[i] = circle2.vertexBuffer[i] + center2;
    }

    this->indexBuffer = circle2.indexBuffer;
    this->colorBuffer = circle2.colorBuffer;

    for (size_t i = 0; i < this->indexBuffer.size(); i += 3) {
        DrawIndexedTriangle(i, pixels);
    }

    this->vertexBuffer.resize(circle3.vertexBuffer.size());
    for (size_t i = 0; i < circle3.vertexBuffer.size(); i++) {
        this->vertexBuffer[i] = circle3.vertexBuffer[i] + center3;
    }

    this->indexBuffer = circle3.indexBuffer;
    this->colorBuffer = circle3.colorBuffer;

    for (size_t i = 0; i < this->indexBuffer.size(); i += 3) {
        DrawIndexedTriangle(i, pixels);
    }
}

void Rasterization::Update() {
    // 애니메이션 구현
}
} // namespace hlab