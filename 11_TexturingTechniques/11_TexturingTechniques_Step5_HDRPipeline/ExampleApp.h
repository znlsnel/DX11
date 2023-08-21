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

    Vector3 m_lightPosition = Vector3(0.0f, 1.0f, 0.0f);
};
} // namespace hlab
