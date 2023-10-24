#include "BillboardModel.h"
#include "GeometryGenerator.h"

#include <numeric>

namespace hlab {

void BillboardModel::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::vector<Vector4> &points,
                                const float width,
                                const ComPtr<ID3D11PixelShader> &pixelShader) {

     //    Model::Initialize(device, context);
    BillboardModel::m_castShadow = false;

    D3D11Utils::CreateVertexBuffer(device, points, m_vertexBuffer);

    m_indexCount = uint32_t(points.size());
    m_vertexShader = Graphics::billboardVS;
    m_geometryShader = Graphics::billboardGS;
    m_inputLayout = Graphics::billboardIL;
    m_pixelShader = pixelShader;

    static int index = 0;
    m_billboardConsts.GetCpu().widthWorld = width;
    m_billboardConsts.GetCpu().index = index;
    index++;

    m_billboardConsts.Initialize(device);
    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Initialize(device);
    m_materialConsts.Initialize(device);

    
    {
        float maxRadius = width + 1e-2f; // 살짝 크게 설정
        m_boundingSphereRadius = maxRadius;
        m_boundingSphere =
            BoundingSphere(m_boundingBox.Center, m_boundingSphereRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            m_boundingSphere.Center, m_boundingSphere.Radius);
        m_boundingSphereMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       m_boundingSphereMesh->vertexBuffer);
        m_boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        m_boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        m_boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      m_boundingSphereMesh->indexBuffer);
        m_boundingSphereMesh->meshConstsGPU = m_meshConsts.Get();
        m_boundingSphereMesh->materialConstsGPU = m_materialConsts.Get();

    }
        // Initialize Bounding Box
  
}

void BillboardModel::Render(ComPtr<ID3D11DeviceContext> &context) {
    
        if (m_hasLifespan && m_appBase->timeSeconds - m_GenerationTime > m_lifespan) {
                m_isVisible = false;
            this->~BillboardModel();
        }

    if (m_isVisible) {
        // 편의상 PSO 설정을 Render()에서 바꾸는 방식
        context->IASetInputLayout(m_inputLayout.Get());
        context->VSSetShader(m_vertexShader.Get(), 0, 0);
        context->PSSetShader(m_pixelShader.Get(), 0, 0);
        ID3D11Buffer *constBuffers[3] = {
            this->m_meshConsts.Get(),
            this->m_materialConsts.Get(),
            this->m_billboardConsts.Get(),
        };
             //   this->m_billboardConsts.Get()}; 
        context->VSSetConstantBuffers(1, 3, constBuffers);
       //context->PSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
       // context->GSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
        context->PSSetConstantBuffers(1, 3, constBuffers);
        context->GSSetConstantBuffers(1, 3, constBuffers);
        context->GSSetShader(m_geometryShader.Get(), 0, 0);
        context->RSSetState(Graphics::solidBothRS.Get());
        //context->OMSetBlendState(Graphics::alphaBS.Get(),
        //                         Graphics::defaultSolidPSO.m_blendFactor,
        //                         0xffffffff);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        UINT stride = sizeof(Vector4); // sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->Draw(m_indexCount, 0);
        context->GSSetShader(NULL, 0, 0);
    }
}

} // namespace hlab