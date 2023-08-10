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

    Mesh circle1;
    Mesh circle2;
    Mesh circle3;
	
	vec3 center1;
    vec3 center2;
    vec3 center3;

    // GPU에서 내부적으로 사용하는 메모리 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;

	// 깊이(화면으로부터의 거리)를 저장하는 버퍼
    vector<float> depthBuffer;
};
} // namespace hlab
