#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "Mesh.h"

namespace hlab {

class Rasterization {
  public:
    Rasterization(const int &width, const int &height);

    // 삼각형을 하나만 그리는 함수 추가
    // Render(.)에서 호출
    void DrawIndexedTriangle(const size_t &startIndex, vector<vec4> &pixels);

    vec2 ProjectWorldToRaster(vec3 point);
    float EdgeFunction(const vec2 &v0, const vec2 &v1,
                       const vec2 &point);
    void Render(vector<vec4> &pixels);
    void Update();

  public:
    int width;
    int height;

    // 여러 가지의 물체를 그릴 준비
    // 같은 물체를 여러 곳에 그릴 때는 새로운 Mesh를 더 만들 필요 없이
    // 한 Mesh를 원하는 위치로 옮겨서 그리면 됩니다.
    vector<shared_ptr<Mesh>> meshes;

    shared_ptr<Mesh> object;

    Light light;

    // GPU에서 내부적으로 사용하는 메모리 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<vec3> normalBuffer;
    vector<size_t> indexBuffer;
    vector<vec3> colorBuffer;
    // vector<vec2> uvBuffer;

    // 깊이(화면으로부터의 거리)를 저장하는 버퍼
    vector<float> depthBuffer;

    // FrontCounterClockwise = False 라고 가정
    // 바꿔말하면 Clockwise가 앞면
    bool cullBackface = true;

    // 정투영(ortho) vs 원근(perspective)투영
    bool usePerspectiveProjection = true;
    bool usePerspectiveCorrectInterpolation = true;

    // 눈과 화면의 거리 (조절 가능)
    float distEyeToScreen = 1.0f;

	// 현재 사용하는 조명 (0: directional, 1: point, 2: spot)
	int lightType = 0;
};
} // namespace hlab
