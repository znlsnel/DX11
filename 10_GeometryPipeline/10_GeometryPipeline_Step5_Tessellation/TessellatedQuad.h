#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "D3D11Utils.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

struct TessellatedQuadConstantData {
    Vector3 eyeWorld;
    float width;
    Matrix model; // Vertex shader
    Matrix view;  // Vertex shader
    Matrix proj;  // Pixel shader
    float time = 0.0f;
    Vector3 padding;
    Vector4 edges = Vector4(1.0f);
    Vector2 inside = Vector2(1.0f);
    Vector2 padding2;
};

static_assert((sizeof(TessellatedQuadConstantData) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class TessellatedQuad {
  public:
    void Initialize(ComPtr<ID3D11Device> &device);

    void Render(ComPtr<ID3D11DeviceContext> &context);

  public:
    TessellatedQuadConstantData m_constantData;    
    ComPtr<ID3D11Buffer> m_constantBuffer;

    // 여기서도 BasicPixelShader 사용
    ComPtr<ID3D11ShaderResourceView> m_diffuseResView;
    ComPtr<ID3D11ShaderResourceView> m_specularResView;

  protected:
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    // Hull shader -> Tessellation stage -> Domain shader
    ComPtr<ID3D11HullShader> m_hullShader;
    ComPtr<ID3D11DomainShader> m_domainShader;

    // ComPtr<ID3D11GeometryShader> m_geometryShader; // 미사용

    uint32_t m_indexCount = 0;

    ComPtr<ID3D11Texture2D> m_texArray;
    ComPtr<ID3D11ShaderResourceView> m_texArraySRV;
};
} // namespace hlab 