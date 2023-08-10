#include "Mesh.h"

namespace hlab {

void Mesh::InitBox() {
    const float scale = 0.7f;

    // ����
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));

    // �Ʒ���
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    // TODO: normal ���⵵ �����ּ���.
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));

    // �ո�
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    // TODO: normal ���⵵ �����ּ���.
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));

    // �޸�
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    // TODO: normal ���⵵ �����ּ���.
    this->normals.push_back(vec3(0.0f, 1.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 1.0f));

    // ����
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    // TODO: normal ���⵵ �����ּ���.
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));

    // ������
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    // TODO: normal ���⵵ �����ּ���.
    this->normals.push_back(vec3(1.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 1.0f, 0.0f));

    this->indices = {
        0,  1,  2,  0,  2,  3,  // ����
        4, 5, 6, 4, 6, 7, //... // �Ʒ���
        1, 7, 6 , 1, 6, 2,//... // �ո�
        0, 3, 5, 0, 5, 4,//... // �޸�
        0, 4, 7, 0, 7, 1,//... // ����
        3, 2, 6, 3, 6, 5//... // ������
    };
}

void Mesh::CopyToBuffer() {

    // CPU �޸𸮿��� GPU �޸𸮷� �����ϴ� ���̶�� ����
    vertexBuffer = vertices;
    indexBuffer = indices;
    normalBuffer = normals;
    // colorBuffer = colors;
    // uvBuffer = textureCoords;

    // CPU �޸𸮴� ����
    vertices.clear();
    normals.clear();
    indices.clear();
    // colors.clear();
    // textureCoords.clear();
}
} // namespace hlab
