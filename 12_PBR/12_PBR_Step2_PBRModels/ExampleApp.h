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
    BasicMeshGroup m_mainObj;
    BoundingSphere m_mainBoundingSphere;
    BasicMeshGroup m_cursorSphere;
    CubeMapping m_cubeMapping;

    bool m_usePerspectiveProjection = true;

    Vector3 m_lightPosition = Vector3(0.0f, 1.0f, 0.0f);
};

} // namespace hlab
