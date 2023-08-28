
#include "BasicMeshGroup.h"
#include "GeometryGenerator.h"

namespace hlab {
void BasicMeshGroup::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::string &basePath,
                                const std::string &filename) {

    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);

    Initialize(device, context, meshes);
}

void BasicMeshGroup::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::vector<MeshData> &meshes) {

    // Sampler �����
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // Mipmap level ���̿��� Point sampling�� �غ�����.
    // sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

    // ConstantBuffer �����
    m_basicVertexConstantData.modelWorld = Matrix();
    m_basicVertexConstantData.view = Matrix();
    m_basicVertexConstantData.projection = Matrix();

    D3D11Utils::CreateConstantBuffer(device, m_basicVertexConstantData,
                                     m_vertexConstantBuffer);
    D3D11Utils::CreateConstantBuffer(device, m_basicPixelConstantData,
                                     m_pixelConstantBuffer);

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
                newMesh->albedoTexture, newMesh->albedoTextureResourceView);
        }

        if (!meshData.normalTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.normalTextureFilename, false,
                newMesh->normalTexture, newMesh->normalTextureResourceView);
        }

        if (!meshData.heightTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.heightTextureFilename, false,
                newMesh->heightTexture, newMesh->heightTextureResourceView);
        }

        if (!meshData.aoTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.aoTextureFilename, false,
                newMesh->aoTexture, newMesh->aoTextureResourceView);
        }

        newMesh->vertexConstantBuffer = m_vertexConstantBuffer;
        newMesh->pixelConstantBuffer = m_pixelConstantBuffer;

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

    // Geometry shader �ʱ�ȭ�ϱ�
    D3D11Utils::CreateGeometryShader(device, L"NormalGS.hlsl",
                                     m_normalGeometryShader);

    D3D11Utils::CreateVertexShaderAndInputLayout(
        device, L"NormalVS.hlsl", basicInputElements, m_normalVertexShader,
        m_basicInputLayout);
    D3D11Utils::CreatePixelShader(device, L"NormalPS.hlsl",
                                  m_normalPixelShader);

    D3D11Utils::CreateConstantBuffer(device, m_normalVertexConstantData,
                                     m_normalVertexConstantBuffer);
}

void BasicMeshGroup::UpdateConstantBuffers(
    ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context) {

    D3D11Utils::UpdateBuffer(device, context, m_basicVertexConstantData,
                             m_vertexConstantBuffer);

    D3D11Utils::UpdateBuffer(device, context, m_basicPixelConstantData,
                             m_pixelConstantBuffer);

    // ��� ���� �׸���
    if (m_drawNormals && m_drawNormalsDirtyFlag) {
        D3D11Utils::UpdateBuffer(device, context, m_normalVertexConstantData,
                                 m_normalVertexConstantBuffer);
        m_drawNormalsDirtyFlag = false;
    }
}

void BasicMeshGroup::Render(ComPtr<ID3D11DeviceContext> &context) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    for (const auto &mesh : m_meshes) {
        context->VSSetShader(m_basicVertexShader.Get(), 0, 0);

        // VertexShader������ Texture ���
        context->VSSetShaderResources(
            0, 1, mesh->heightTextureResourceView.GetAddressOf());
        context->VSSetSamplers(0, 1, m_samplerState.GetAddressOf());
        context->VSSetConstantBuffers(
            0, 1, mesh->vertexConstantBuffer.GetAddressOf());

        context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
        context->PSSetShader(m_basicPixelShader.Get(), 0, 0);

        // ��ü �������� �� �������� �ؽ��� ���
        // vector<ID3D11ShaderResourceView *> resViews = {
        //    m_diffuseResView.Get(), m_specularResView.Get(),
        //    mesh->albedoTextureResourceView.Get(),
        //    mesh->normalTextureResourceView.Get(),
        //    mesh->aoTextureResourceView.Get()};
        vector<ID3D11ShaderResourceView *> resViews = {
            NULL, NULL, mesh->albedoTextureResourceView.Get(),
            mesh->normalTextureResourceView.Get(),
            mesh->aoTextureResourceView.Get()};
        context->PSSetShaderResources(0, UINT(resViews.size()),
                                      resViews.data());

        context->PSSetConstantBuffers(0, 1,
                                      mesh->pixelConstantBuffer.GetAddressOf());

        context->IASetInputLayout(m_basicInputLayout.Get());
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(mesh->m_indexCount, 0, 0);

        if (m_drawNormals) {
            // ���� VertexBuffer ���
            context->VSSetShader(m_normalVertexShader.Get(), 0, 0);
            ID3D11Buffer *pptr[2] = {m_vertexConstantBuffer.Get(),
                                     m_normalVertexConstantBuffer.Get()};
            context->GSSetConstantBuffers(0, 2, pptr);
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

    m_basicVertexConstantData.modelWorld = modelWorldRow.Transpose();
    m_basicVertexConstantData.invTranspose = m_invTransposeRow.Transpose();
}
} // namespace hlab