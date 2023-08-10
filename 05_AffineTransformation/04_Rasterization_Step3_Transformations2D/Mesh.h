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
    void InitCircle(const vec3 &center, const float &radius,
                    const int &numTriangles);

  public:
    vector<vec3> vertices;
    vector<vec3> colors;
    vector<size_t> indices;
};
} // namespace hlab