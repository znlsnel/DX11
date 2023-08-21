﻿#pragma once

#include "BasicConstantData.h"
#include "D3D11Utils.h"
#include "Mesh.h"
#include "MeshData.h"

namespace hlab {

class BasicMeshGroup {
  public:
    BasicMeshGroup() {}
    BasicMeshGroup(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context,
                   const std::string &basePath, const std::string &filename);
    BasicMeshGroup(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context,
                   const std::vector<MeshData> &meshes);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::string &basePath, const std::string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<MeshData> &meshes);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11DeviceContext> &context,
                ComPtr<ID3D11Buffer> &eyeViewProjCB, bool useEnv);

    void UpdateModelWorld(const Matrix &modelToWorldRow);

  public:
    // Model/World 행렬: 원점에 정의된 모델을 월드 좌표계에서 그려질 위치로
    // 변환해주는 행렬
    // 보다 구체적으로는 Model to World 변환 행렬이라고 할 수 있습니다.
    // DirectX(언리얼 엔진)에서는 World 행렬이라고 부릅니다.
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
    // https://gamedev.stackexchange.com/questions/194929/difference-between-world-matrix-and-model-matrix
    // https://jsantell.com/model-view-projection/
    Matrix m_modelWorldRow = Matrix();
    Matrix m_invTransposeRow = Matrix();

    // ExampleApp::Update()에서 접근
    BasicVertexConstData m_basicVertexConstData;
    BasicPixelConstData m_basicPixelConstData;

    // ExampleApp:Initialize()에서 접근
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

    // GUI에서 업데이트 할 때 사용
    NormalVertexConstantData m_normalVertexConstData;
    bool m_drawNormalsDirtyFlag = true;
    bool m_drawNormals = false;

    std::vector<shared_ptr<Mesh>> m_meshes; // Mipmaps 생성을 위해 임시로

  private:
    ComPtr<ID3D11VertexShader> m_basicVertexShader;
    ComPtr<ID3D11PixelShader> m_basicPixelShader;
    ComPtr<ID3D11InputLayout> m_basicInputLayout;

    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11SamplerState> m_clampSamplerState;

    ComPtr<ID3D11Buffer> m_vertexConstBuffer;
    ComPtr<ID3D11Buffer> m_pixelConstBuffer;

    // 메쉬의 노멀 벡터 그리기
    ComPtr<ID3D11GeometryShader> m_normalGeometryShader;
    ComPtr<ID3D11VertexShader> m_normalVertexShader;
    ComPtr<ID3D11PixelShader> m_normalPixelShader;

    // shared_ptr<Mesh> m_normalLines;

    ComPtr<ID3D11Buffer> m_normalVertexConstBuffer;
    ComPtr<ID3D11Buffer> m_normalPixelConstBuffer;
};

} // namespace hlab
