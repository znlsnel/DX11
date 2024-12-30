#pragma once

#include "AppBase.h"
#include "Model.h"
#include "StructuredBuffer.h"

namespace hlab {

class Ex1404_StructuredBuffer : public AppBase {
  public:
    struct Particle {
        Vector3 position;
        Vector3 color;
    };

    struct CSData {
        float velocity = 0.0f;
        Vector3 dummy;
    };

    Ex1404_StructuredBuffer();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    // Vertex Buffer (Compute Shader에서도 사용)
    StructuredBuffer<Particle> m_particles;
    CSData m_CSConstsCPU; 
    
    ComPtr<ID3D11Buffer> m_CSConstsGPU;

    // Shaders
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11ComputeShader> m_computeShader;
};

} // namespace hlab