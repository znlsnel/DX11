#pragma once

#include "D3D11Utils.h"

namespace hlab {

// ����: DirectX_Graphic-Samples �̴Ͽ���
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/PipelineState.h

// GraphicsPSO�� ������ �� �ξ� �ܼ��մϴ�.
class ComputePSO {
  public:
    void operator=(const ComputePSO &pso) {
        m_computeShader = pso.m_computeShader;
    };

  public:
    ComPtr<ID3D11ComputeShader> m_computeShader;
};

} // namespace hlab