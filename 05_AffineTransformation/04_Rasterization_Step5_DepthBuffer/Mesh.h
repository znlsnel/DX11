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
    void InitCircle(const float &radius, const int &numTriangles, const vec3& color);

  public:
	// CPU 메모리의 기하 정보
    vector<vec3> vertices;
    vector<size_t> indices;
    vector<vec3> colors;

	// GPU 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
};
} // namespace hlab