#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "AppBase.h"
#include "Model.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

struct BillboardConsts {
    float widthWorld;
    Vector3 directionWorld;
    int index = 0;
    Vector3 dummy;
};

static_assert((sizeof(BillboardConsts) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class BillboardModel : public Model {

  public:
    BillboardModel(AppBase* appBase, float lifespan = -1.0f) {
        m_appBase = appBase;
        m_lifespan = lifespan;
        if (lifespan > 0) {
            m_hasLifespan = true;
                m_GenerationTime = m_appBase->timeSeconds;
        
        }
    };

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<Vector4> &points, const float width,
                    const ComPtr<ID3D11PixelShader> &pixelShader);

    void Render(ComPtr<ID3D11DeviceContext> &context) override;
    void RenderNormals(ComPtr<ID3D11DeviceContext> &context) override{};
    void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context) override{};
    void
    RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context) override{};

    GraphicsPSO &GetPSO(const bool wired) override {
        // return wired ? Graphics::defaultWirePSO : fireballPSO;
        return Graphics::defaultWirePSO;
    }

  public:
    ConstantBuffer<BillboardConsts> m_billboardConsts;
    vector<ID3D11ShaderResourceView *> resViews;
    AppBase* m_appBase;

    bool m_hasLifespan = false;
    bool useOtherShaderResource = false; 
    float m_lifespan = 3.0f;
    float m_GenerationTime = 0.0f;

    ComPtr<ID3D11GeometryShader> m_geometryShader;
    ComPtr<ID3D11Buffer> m_foliageConstsGPU; 
  protected:
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    uint32_t m_indexCount = 0;
};

} // namespace hlab