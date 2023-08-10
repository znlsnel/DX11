#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

// 참고: Rasterization: a Practical Implementation
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation

namespace hlab {

using namespace glm;
using namespace std;

// Example.h 안에도 Vertex라는 구조체가 정의되어 있습니다.
// 이번 실습에서는 아래의 MyVertex와 MyTriangle을 사용합니다.
struct MyVertex {
    vec3 pos;
    vec3 color;
};

struct MyTriangle {
    MyVertex v0, v1, v2;
};

class Rasterization {
  public:
    Rasterization(const int &width, const int &height);

    vec2 ProjectWorldToRaster(vec3 point);
    float EdgeFunction(const vec2 &v0, const vec2 &v1, const vec2 &point);
    void Render(vector<vec4> &pixels);
    void Update();

  public:
    int width;
    int height;
    MyTriangle triangle;
};
} // namespace hlab
