#include "Mesh.h"

namespace hlab {
void Mesh::InitCircle(const float &radius, const int &numTriangles,
                      const vec3 &color) {

    const vec3 center = vec3(0.0f); // 중심은 원점으로 고정

    this->vertices.reserve(numTriangles + 1);
    this->colors.reserve(this->vertices.size());
    this->indices.reserve(numTriangles * 3);

    this->vertices.push_back(center);
    this->colors.push_back(color);

    const auto kTwoPi = 2.0f * 3.141592f;
    const auto deltaTheta = kTwoPi / float(numTriangles);

    for (float theta = 0.0f; theta < kTwoPi; theta += deltaTheta) {
        this->vertices.push_back(center +
                                 vec3(cos(theta), sin(theta), 0.0f) * radius);
        
        // 모든 버텍스를 동일한 색으로 설정할 경우에는
		// colors를 제거할 수도 있으나 앞 강의와의 연결성을 위해 남겨둠
        this->colors.push_back(color); 
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