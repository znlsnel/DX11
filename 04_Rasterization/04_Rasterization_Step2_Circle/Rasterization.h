#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

// 참고: Rasterization: a Practical Implementation
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation

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

    vector<vec3> vertices;
    vector<vec3> colors;
    vector<size_t> indices;
};
} // namespace hlab
