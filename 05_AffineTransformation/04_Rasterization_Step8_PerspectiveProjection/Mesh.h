#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace hlab {

using namespace glm;
using namespace std;

// 폴리곤 메쉬를 다루는 클래스
class Mesh {
  public:
    void InitCircle(const float &radius, const int &numTriangles,
                    const vec3 &color);
    void InitSquare(const float &width, const float &height, const vec3 &color);
    void CopyToBuffer();

  public:
    // CPU 메모리의 기하 정보
    vector<vec3> vertices;
    vector<size_t> indices;
    vector<vec3> colors;
    vector<vec2> textureCoords; // Texture Coordinates

    // GPU 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
    vector<vec2> uvBuffer;

    // 모든 버텍스에 공통으로 적용되는 변환(Transformations)
    vec3 scale = vec3(1.0f);
    vec3 translation = vec3(0.0f);
    float rotationX = 0.0f;
    float rotationZ = 0.0f;
};
} // namespace hlab