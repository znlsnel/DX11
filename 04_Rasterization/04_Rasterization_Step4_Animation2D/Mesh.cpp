#include "Mesh.h"

namespace hlab {
void Mesh::InitCircle(const float &radius, const int &numTriangles,
                      const vec3 &color) {

    const vec3 center = vec3(0.0f); // �߽��� �������� ����

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
        
        // ��� ���ؽ��� ������ ������ ������ ��쿡��
		// colors�� ������ ���� ������ �� ���ǿ��� ���Ἲ�� ���� ���ܵ�
        this->colors.push_back(color); 
    }

    for (size_t i = 0; i < numTriangles; i++) {

        // �ð� �������� ��ġ
        this->indices.push_back(0);

        // ù ���ؽ��� �������� ��Ȱ��
        if (i == numTriangles - 1) {
            this->indices.push_back(1);
        } else {
            this->indices.push_back(i + 2);
        }

        this->indices.push_back(i + 1);
    }
}
} // namespace hlab