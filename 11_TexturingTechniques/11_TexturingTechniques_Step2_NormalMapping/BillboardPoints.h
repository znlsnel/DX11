#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "D3D11Utils.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

struct BillboardPointsConstantData {
    Vector3 eyeWorld;
    float width;
    Matrix model; // Vertex shader
    Matrix view;  // Vertex shader
    Matrix proj;  // Pixel shader
    float time = 0.0f;
    Vector3 padding;
};

static_assert((sizeof(BillboardPointsConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class BillboardPoints {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<Vector4> &points, const float width,
                    const std::wstring pixelShaderFilename =
                        L"BillboardPointsPixelShader.hlsl",
                    std::vector<std::string> filenames = {});

    void Render(ComPtr<ID3D11DeviceContext> &context);

  public:
    BillboardPointsConstantData m_constantData;

    // 편의상 ConstantBuffer를 하나만 사용
    ComPtr<ID3D11Buffer> m_constantBuffer;

  protected:
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11GeometryShader> m_geometryShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    uint32_t m_indexCount = 0;

    ComPtr<ID3D11Texture2D> m_texArray;
    ComPtr<ID3D11ShaderResourceView> m_texArraySRV;
};
} // namespace hlab