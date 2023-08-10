#include "Mesh.h"

namespace hlab {

void Mesh::InitBox() {
    const float scale = 0.7f;

    // 윗면
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, 1.0f, 0.0f));

    // 아랫면
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    this->normals.push_back(vec3(0.0f, -1.0f, 0.0f));

    // 앞면
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, -1.0f));

    // 뒷면
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));
    this->normals.push_back(vec3(0.0f, 0.0f, 1.0f));

    // 왼쪽
    this->vertices.push_back(vec3(-1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(-1.0f, -1.0f, -1.0f) * scale);
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(-1.0f, 0.0f, 0.0f));

    // 오른쪽
    this->vertices.push_back(vec3(1.0f, -1.0f, 1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, -1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, -1.0f) * scale);
    this->vertices.push_back(vec3(1.0f, 1.0f, 1.0f) * scale);
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));
    this->normals.push_back(vec3(1.0f, 0.0f, 0.0f));

    this->indices = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };
}

void Mesh::CopyToBuffer() {

    // CPU 메모리에서 GPU 메모리로 복사하는 것이라고 생각
    vertexBuffer = vertices;
    indexBuffer = indices;
    normalBuffer = normals;
    // colorBuffer = colors;
    // uvBuffer = textureCoords;

    // CPU 메모리는 삭제
    vertices.clear();
    normals.clear();
    indices.clear();
    // colors.clear();
    // textureCoords.clear();
}
} // namespace hlab
