#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "Mesh.h"

namespace hlab {

using namespace glm;
using namespace std;

class Rasterization {
  public:
    Rasterization(const int &width, const int &height);

    // 삼각형을 하나만 그리는 함수 추가
    // Render(.)에서 호출
    void DrawIndexedTriangle(const size_t &startIndex, vector<vec4> &pixels);

    vec2 ProjectWorldToRaster(vec3 point);
    float EdgeFunction(const vec2 &v0, const vec2 &v1, const vec2 &point);
    void Render(vector<vec4> &pixels);
    void Update();

  public:
    int width;
    int height;

    Mesh circle; // 원의 기하 정보를 담고 있는 메쉬

    vec3 translation1 = vec3(0.0f); // 첫 번째 이동 물체의 이동
    vec3 translation2 = vec3(0.0f); // 두 번째 이동 물체의 이동
    float rotation1 = 0.0f;         // 첫 번째 회전
    float rotation2 = 0.0f;         // 두 번째 회전
    float scaleX = 1.0f;            // 스케일은 한 번만 구현
    float scaleY = 1.0f;

    vector<vec3> vertexBuffer;
    vector<vec3> colorBuffer;
    vector<size_t> indexBuffer;
};
} // namespace hlab
