#pragma once

#include "D3D11Utils.h"

namespace hlab {

// ����: DirectX_Graphic-Samples �̴Ͽ���
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/PipelineState.h

// ����: D3D12_GRAPHICS_PIPELINE_STATE_DESC
// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_graphics_pipeline_state_desc

// PipelineStateObject: �������� �� Context�� ���¸� ��� ���������� ����
// ComputePSO�� ������ ����

class GraphicsPSO {
  public:
    void operator=(const GraphicsPSO &pso);
    void SetBlendFactor(const float blendFactor[4]);

  public:
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11HullShader> m_hullShader;
    ComPtr<ID3D11DomainShader> m_domainShader;
    ComPtr<ID3D11GeometryShader> m_geometryShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    ComPtr<ID3D11RasterizerState> m_rasterizerState;

    float m_blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    UINT m_stencilRef = 0;

    D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology =
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

} // namespace hlab