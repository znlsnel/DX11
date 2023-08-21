
#include "BasicMeshGroup.h"
#include "GeometryGenerator.h"

namespace hlab {

BasicMeshGroup::BasicMeshGroup(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context,
                               const std::string &basePath,
                               const std::string &filename, bool invertNormals) {
    this->Initialize(device, context, basePath, filename, invertNormals);
}

BasicMeshGroup::BasicMeshGroup(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context,
                               const std::vector<MeshData> &meshes) {
    this->Initialize(device, context, meshes);
}

void BasicMeshGroup::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::string &basePath,
                                const std::string &filename,
                                bool invertNormals) {

    auto meshes =
        GeometryGenerator::ReadFromFile(basePath, filename, invertNormals);

    Initialize(device, context, meshes);
}

void BasicMeshGroup::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::vector<MeshData> &meshes) {

    // Sampler 만들기
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // Mipmap level 사이에서 Point sampling도 해보세요.
    // sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    device->CreateSamplerState(&sampDesc, m_clampSamplerState.GetAddressOf());

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.BorderColor[0] = 100.0f;
    device->CreateSamplerState(&sampDesc, m_borderSamplerState.GetAddressOf());

    // ConstantBuffer 만들기
    m_basicVertexConstData.modelWorld = Matrix();

    D3D11Utils::CreateConstBuffer(device, m_basicVertexConstData,
                                  m_vertexConstBuffer);
    D3D11Utils::CreateConstBuffer(device, m_basicPixelConstData,
                                  m_pixelConstBuffer);

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       newMesh->vertexBuffer);
        newMesh->m_indexCount = UINT(meshData.indices.size());
        newMesh->m_vertexCount = UINT(meshData.vertices.size());

        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      newMesh->indexBuffer);

        if (!meshData.albedoTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.albedoTextureFilename, true,
                newMesh->albedoTexture, newMesh->albedoSRV);
            m_basicPixelConstData.useAlbedoMap = true;
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.emissiveTextureFilename, true,
                newMesh->emissiveTexture, newMesh->emissiveSRV);
            m_basicPixelConstData.useEmissiveMap = true;
        }

        if (!meshData.normalTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.normalTextureFilename, false,
                newMesh->normalTexture, newMesh->normalSRV);
            m_basicPixelConstData.useNormalMap = true;
        }

        if (!meshData.heightTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.heightTextureFilename, false,
                newMesh->heightTexture, newMesh->heightSRV);
            m_basicVertexConstData.useHeightMap = true;
        }

        if (!meshData.aoTextureFilename.empty()) {
            D3D11Utils::CreateTexture(device, context,
                                      meshData.aoTextureFilename, false,
                                      newMesh->aoTexture, newMesh->aoSRV);
            m_basicPixelConstData.useAOMap = true;
        }

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {
            D3D11Utils::CreateMetallicRoughnessTexture(
                device, context, meshData.metallicTextureFilename,
                meshData.roughnessTextureFilename,
                newMesh->metallicRoughnessTexture,
                newMesh->metallicRoughnessSRV);
        }

        if (!meshData.metallicTextureFilename.empty()) {
            m_basicPixelConstData.useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            m_basicPixelConstData.useRoughnessMap = true;
        }

        newMesh->vertexConstBuffer = m_vertexConstBuffer;
        newMesh->pixelConstBuffer = m_pixelConstBuffer;

        this->m_meshes.push_back(newMesh);
    }

    vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    D3D11Utils::CreateVertexShaderAndInputLayout(
        device, L"BasicVS.hlsl", basicInputElements, m_basicVertexShader,
        m_basicInputLayout);

    D3D11Utils::CreatePixelShader(device, L"BasicPS.hlsl", m_basicPixelShader);

    // Geometry shader 초기화하기
    D3D11Utils::CreateGeometryShader(device, L"NormalGS.hlsl",
                                     m_normalGeometryShader);

    D3D11Utils::CreateVertexShaderAndInputLayout(
        device, L"NormalVS.hlsl", basicInputElements, m_normalVertexShader,
        m_basicInputLayout);
    D3D11Utils::CreatePixelShader(device, L"NormalPS.hlsl",
                                  m_normalPixelShader);

    D3D11Utils::CreateConstBuffer(device, m_normalVertexConstData,
                                  m_normalVertexConstBuffer);
}

void BasicMeshGroup::UpdateConstantBuffers(
    ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context) {

    D3D11Utils::UpdateBuffer(device, context, m_basicVertexConstData,
                             m_vertexConstBuffer);

    D3D11Utils::UpdateBuffer(device, context, m_basicPixelConstData,
                             m_pixelConstBuffer);

    // 노멀 벡터 그리기
    if (m_drawNormals && m_drawNormalsDirtyFlag) {
        D3D11Utils::UpdateBuffer(device, context, m_normalVertexConstData,
                                 m_normalVertexConstBuffer);
        m_drawNormalsDirtyFlag = false;
    }
}

void BasicMeshGroup::Render(ComPtr<ID3D11DeviceContext> &context,
                            ComPtr<ID3D11Buffer> &eyeViewProjCB, bool useEnv) {

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    for (const auto &mesh : m_meshes) {
        context->VSSetShader(m_basicVertexShader.Get(), 0, 0);

        // VertexShader에서도 Texture 사용
        context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());
        context->VSSetSamplers(0, 1, m_samplerState.GetAddressOf());

        vector<ID3D11Buffer *> vertexCB = {mesh->vertexConstBuffer.Get(),
                                           eyeViewProjCB.Get()};
        context->VSSetConstantBuffers(0, UINT(vertexCB.size()),
                                      vertexCB.data());

        vector<ID3D11SamplerState *> samplers = {m_samplerState.Get(),
                                                 m_clampSamplerState.Get(),
                                                 m_borderSamplerState.Get()};
        context->PSSetSamplers(0, UINT(samplers.size()), samplers.data());
        context->PSSetShader(m_basicPixelShader.Get(), 0, 0);

        // 물체 렌더링할 때 여러가지 텍스춰 사용
        vector<ID3D11ShaderResourceView *> resViews = {
            useEnv ? m_specularSRV.Get() : NULL,
            useEnv ? m_irradianceSRV.Get() : NULL,
            m_brdfSRV.Get(),
            mesh->albedoSRV.Get(),
            mesh->normalSRV.Get(),
            mesh->aoSRV.Get(),
            mesh->metallicRoughnessSRV.Get(),
            mesh->emissiveSRV.Get(),
            m_shadowSRV.Get() // <- 그림자 추가
        };
        context->PSSetShaderResources(0, UINT(resViews.size()),
                                      resViews.data());

        vector<ID3D11Buffer *> pixelCB = {
            mesh->pixelConstBuffer.Get(), eyeViewProjCB.Get(),
            m_lightConstBuffer.Get(), // <- 그림자 처리에 사용
        };
        context->PSSetConstantBuffers(0, UINT(pixelCB.size()), pixelCB.data());

        context->IASetInputLayout(m_basicInputLayout.Get());
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(mesh->m_indexCount, 0, 0);

        if (m_drawNormals) {
            // 같은 VertexBuffer 사용
            context->VSSetShader(m_normalVertexShader.Get(), 0, 0);
            vector<ID3D11Buffer *> gCB = {m_vertexConstBuffer.Get(),
                                          eyeViewProjCB.Get(),
                                          m_normalVertexConstBuffer.Get()};
            context->GSSetConstantBuffers(0, UINT(gCB.size()), gCB.data());
            context->GSSetShader(m_normalGeometryShader.Get(), 0, 0);
            context->PSSetShader(m_normalPixelShader.Get(), 0, 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
            context->Draw(mesh->m_vertexCount, 0);
            context->GSSetShader(NULL, 0, 0);
        }
    }
}

void BasicMeshGroup::UpdateModelWorld(const Matrix &modelWorldRow) {
    this->m_modelWorldRow = modelWorldRow;
    this->m_invTransposeRow = modelWorldRow;
    m_invTransposeRow.Translation(Vector3(0.0f));
    m_invTransposeRow = m_invTransposeRow.Invert().Transpose();

    m_basicVertexConstData.modelWorld = modelWorldRow.Transpose();
    m_basicVertexConstData.invTranspose = m_invTransposeRow.Transpose();
}

} // namespace hlab