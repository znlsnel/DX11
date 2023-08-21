#pragma once

#include <memory>

#include "Model.h"

// ����: DirectX-Graphics-Sampels
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

namespace hlab {

class ModelInstance {
  public:
  public:
    std::shared_ptr<const Model> m_model;
    // UploadBuffer m_MeshConstantsCPU;
    // ByteAddressBuffer m_MeshConstantsGPU;
    // UniformTransform locator (��ġ ã���� ���°�, bounding box ��)
};

} // namespace hlab