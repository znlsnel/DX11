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

    Mesh sun;   // 태양
    Mesh earth; // 지구(태양 주위를 돈다)
    Mesh moon;  // 달(지구의 위성)

    float earthAngle = 0.0f; // 태양에 대한 지구의 공전 각도
    float earthAngularVelocity = 0.3f; // 각속도
    float moonAngle = 0.0f;            // 지구에 대한 달의 공전 각도
    float moonAngularVelocity = 1.0f;  // 각속도

    float distSunToEarth = 0.5f;  // 태양과 지구의 거리
    float distEarthToMoon = 0.1f; // 지구와 달의 거리

    // GPU에서 내부적으로 사용하는 메모리 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
};
} // namespace hlab
