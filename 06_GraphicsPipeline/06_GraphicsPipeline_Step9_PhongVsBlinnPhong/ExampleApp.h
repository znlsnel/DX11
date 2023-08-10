#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>
#include <vector>

#include "AppBase.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// 이 예제에서 사용하는 Vertex 정의
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    // Vector3 color;
};

// 재질
struct Material {
    Vector3 ambient = Vector3(0.1f);  // 12
    float shininess = 1.0f;           // 4
    Vector3 diffuse = Vector3(0.5f);  // 12
    float dummy1;                     // 4
    Vector3 specular = Vector3(0.5f); // 12
    float dummy2;                     // 4
};

// 조명
struct Light {
    // 순서와 크기 관계 주의 (16 바이트 패딩)
    Vector3 strength = Vector3(1.0f);              // 12
    float fallOffStart = 0.0f;                     // 4
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 12
    float fallOffEnd = 10.0f;                      // 4
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 12
    float spotPower = 1.0f;                        // 4
};

// 이 예제에서 ConstantBuffer로 보낼 데이터
struct VertexConstantBuffer {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

// 주의:
// For a constant buffer (BindFlags of D3D11_BUFFER_DESC set to
// D3D11_BIND_CONSTANT_BUFFER), you must set the ByteWidth value of
// D3D11_BUFFER_DESC in multiples of 16, and less than or equal to
// D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createbuffer

static_assert((sizeof(VertexConstantBuffer) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

#define MAX_LIGHTS 3

struct PixelConstantBuffer {
    Vector3 eyeWorld;        // 12
    bool useTexture;         // 4
    Material material;       // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS
    bool useBlinnPhong = true;
    Vector3 dummy;
};

static_assert((sizeof(PixelConstantBuffer) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class ExampleApp : public AppBase {
  public:
    ExampleApp();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    ComPtr<ID3D11VertexShader> m_colorVertexShader;
    ComPtr<ID3D11PixelShader> m_colorPixelShader;
    ComPtr<ID3D11InputLayout> m_colorInputLayout;

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
    ComPtr<ID3D11Buffer> m_pixelShaderConstantBuffer;
    UINT m_indexCount;

    // Texturing
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_textureResourceView;
    ComPtr<ID3D11Texture2D> m_texture2;
    ComPtr<ID3D11ShaderResourceView> m_textureResourceView2;
    ComPtr<ID3D11SamplerState> m_samplerState;

    VertexConstantBuffer m_vertexConstantBufferData;
    PixelConstantBuffer m_pixelConstantBufferData;

    bool m_usePerspectiveProjection = true;
    Vector3 m_modelTranslation = Vector3(0.0f);
    Vector3 m_modelRotation = Vector3(0.0f);
    Vector3 m_modelScaling = Vector3(0.5f);
    float m_viewRot = 0.0f;

    float m_projFovAngleY = 70.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;

    int m_lightType = 0;
    Light m_lightFromGUI;
    float m_materialDiffuse = 1.0f;
    float m_materialSpecular = 1.0f;
};
} // namespace hlab