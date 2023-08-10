#include "Mesh.h"

namespace hlab {
void Mesh::InitCircle(const vec3 &center, const float &radius,
                             const int &numTriangles) {

    this->vertices.reserve(numTriangles + 1);
    this->colors.reserve(this->vertices.size());
    this->indices.reserve(numTriangles * 3);

    // 중심 버텍스의 위치와 색
    this->vertices.push_back(center);
    this->colors.push_back(vec3(1.0f, 0.0f, 0.0f));

    // 라디안(Radian) 2*PI는 360도를 의미합니다.
    // https://google.github.io/styleguide/cppguide.html#Constant_Names
    const auto kTwoPi = 2.0f * 3.141592f;
    const auto deltaTheta = kTwoPi / float(numTriangles);

    // 가장자리 버텍스 정의
    for (float theta = 0.0f; theta < kTwoPi; theta += deltaTheta) {
        this->vertices.push_back(center +
                                 vec3(cos(theta), sin(theta), 0.0f) * radius);
        this->colors.push_back(vec3(0.0f, 0.0f, 1.0f));
    }

    for (size_t i = 0; i < numTriangles; i++) {

        // 시계 방향으로 배치
        this->indices.push_back(0);

        // 첫 버텍스를 마지막에 재활용
        if (i == numTriangles - 1) {
            this->indices.push_back(1);
        } else {
            this->indices.push_back(i + 2);
        }

        this->indices.push_back(i + 1);
    }
}
} // namespace hlab