#pragma once

#include <algorithm>
#include <iostream>
#include <memory>

#include "AppBase.h"
#include "BasicMeshGroup.h"
#include "BillboardPoints.h"
#include "CubeMapping.h"
#include "GeometryGenerator.h"
#include "ImageFilter.h"
#include "Light.h"
#include "TessellatedQuad.h"

namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

class ExampleApp : public AppBase {
  public:
    ExampleApp();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void BuildFilters();

  protected:
    TessellatedQuad m_tesselatedQuad;
    BillboardPoints m_billboardPoints;
    // BillboardPoints m_fireballs;
    BasicMeshGroup m_mainSphere;
    BoundingSphere m_mainBoundingSphere;
    BasicMeshGroup m_cursorSphere;
    BasicMeshGroup m_meshGroupGround;
    CubeMapping m_cubeMapping;

    bool m_usePerspectiveProjection = true;

    // 블룸 효과 옵션
    int m_dirtyflag = 1; // 처음에 한 번 실행
    int m_down = 4;
    int m_repeat = 3;
    float m_threshold = 0.0f;
    float m_strength = 0.6f;

    // 후처리 필터
    std::vector<shared_ptr<ImageFilter>> m_filters;
};
} // namespace hlab