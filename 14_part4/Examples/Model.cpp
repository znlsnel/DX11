
#include "Model.h"
#include "GeometryGenerator.h"
#include <filesystem>

namespace hlab {

using namespace std;
using namespace DirectX;

Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename) {
    Initialize(device, context, basePath, filename);
}

Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::vector<MeshData> &meshes) {
    Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context) {
    std::cout << "Model::Initialize(ComPtr<ID3D11Device> &device, "
                 "ComPtr<ID3D11DeviceContext> &context) was not implemented."
              << std::endl;
    exit(-1);
}

void Model::InitMeshBuffers(ComPtr<ID3D11Device> &device,
                            const MeshData &meshData,
                            shared_ptr<Mesh> &newMesh) {

    D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                   newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                  newMesh->indexBuffer);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::string &basePath,
                       const std::string &filename) {
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(device, context, meshes);
}

BoundingBox GetBoundingBox(const vector<hlab::Vertex> &vertices) {

    if (vertices.size() == 0)
        return BoundingBox();

    Vector3 minCorner = vertices[0].position;
    Vector3 maxCorner = vertices[0].position;

    for (size_t i = 1; i < vertices.size(); i++) {
        minCorner = Vector3::Min(minCorner, vertices[i].position);
        maxCorner = Vector3::Max(maxCorner, vertices[i].position);
    }

    Vector3 center = (minCorner + maxCorner) * 0.5f;
    Vector3 extents = maxCorner - center;

    return BoundingBox(center, extents);
}

void ExtendBoundingBox(const BoundingBox &inBox, BoundingBox &outBox) {

    Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
    Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

    minCorner = Vector3::Min(minCorner,
                             Vector3(outBox.Center) - Vector3(outBox.Extents));
    maxCorner = Vector3::Max(maxCorner,
                             Vector3(outBox.Center) + Vector3(outBox.Extents));

    outBox.Center = (minCorner + maxCorner) * 0.5f;
    outBox.Extents = maxCorner - outBox.Center;
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const vector<MeshData> &meshes) {

    // �Ϲ������δ� Mesh���� m_mesh/materialConsts�� ���� ���� ����
    // ���⼭�� �� Model ���� ���� Mesh���� Consts�� ��� ����

    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Initialize(device);
    m_materialConsts.Initialize(device);

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();

        InitMeshBuffers(device, meshData, newMesh);

        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                } else {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename, true,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                }

                m_materialConsts.GetCpu().useAlbedoMap = true;
            } else {
                cout << meshData.albedoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            if (filesystem::exists(meshData.emissiveTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.emissiveTextureFilename, true,
                    newMesh->emissiveTexture, newMesh->emissiveSRV);
                m_materialConsts.GetCpu().useEmissiveMap = true;
            } else {
                cout << meshData.emissiveTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.normalTextureFilename, false,
                    newMesh->normalTexture, newMesh->normalSRV);
                m_materialConsts.GetCpu().useNormalMap = true;
            } else {
                cout << meshData.normalTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.heightTextureFilename, false,
                    newMesh->heightTexture, newMesh->heightSRV);
                m_meshConsts.GetCpu().useHeightMap = true;
            } else {
                cout << meshData.heightTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                D3D11Utils::CreateTexture(device, context,
                                          meshData.aoTextureFilename, false,
                                          newMesh->aoTexture, newMesh->aoSRV);
                m_materialConsts.GetCpu().useAOMap = true;
            } else {
                cout << meshData.aoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        // GLTF ������� Metallic�� Roughness�� �� �ؽ��翡 ����
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {

            if (filesystem::exists(meshData.metallicTextureFilename) &&
                filesystem::exists(meshData.roughnessTextureFilename)) {

                D3D11Utils::CreateMetallicRoughnessTexture(
                    device, context, meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    newMesh->metallicRoughnessTexture,
                    newMesh->metallicRoughnessSRV);
            } else {
                cout << meshData.metallicTextureFilename << " or "
                     << meshData.roughnessTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.metallicTextureFilename.empty()) {
            m_materialConsts.GetCpu().useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            m_materialConsts.GetCpu().useRoughnessMap = true;
        }

        newMesh->meshConstsGPU = m_meshConsts.Get();
        newMesh->materialConstsGPU = m_materialConsts.Get();

        this->m_meshes.push_back(newMesh);
    }

    // Initialize Bounding Box
    {
        m_boundingBox = GetBoundingBox(meshes[0].vertices);
        for (size_t i = 1; i < meshes.size(); i++) {
            auto bb = GetBoundingBox(meshes[i].vertices);
            ExtendBoundingBox(bb, m_boundingBox);
        }
        auto meshData = GeometryGenerator::MakeWireBox(
            m_boundingBox.Center,
            Vector3(m_boundingBox.Extents) + Vector3(1e-3f));
        m_boundingBoxMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       m_boundingBoxMesh->vertexBuffer);
        m_boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        m_boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        m_boundingBoxMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      m_boundingBoxMesh->indexBuffer);
        m_boundingBoxMesh->meshConstsGPU = m_meshConsts.Get();
        m_boundingBoxMesh->materialConstsGPU = m_materialConsts.Get();
    }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto &mesh : meshes) {
            for (auto &v : mesh.vertices) {
                maxRadius = std::max(
                    (Vector3(m_boundingBox.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // ��¦ ũ�� ����
        m_boundingSphere = BoundingSphere(m_boundingBox.Center, maxRadius);
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
}

void Model::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        m_meshConsts.Upload(context);
        m_materialConsts.Upload(context);
    }
}

GraphicsPSO &Model::GetPSO(const bool wired) {

       if (useTessellation) {
        return wired ? Graphics::terrainWirePSO : Graphics::terrainSolidPSO;
    }
    return wired ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO;
}

GraphicsPSO &Model::GetDepthOnlyPSO() { return Graphics::depthOnlyPSO; }

GraphicsPSO &Model::GetReflectPSO(const bool wired) {
    return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
} 


void Model::Render(ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        for (const auto &mesh : m_meshes) {

            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            context->VSSetConstantBuffers(1, 2, constBuffers);

            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());

            // ��ü �������� �� �������� �ؽ��� ��� (t0 ���ͽ���)
            vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};
            context->PSSetShaderResources(0, // register(t0)
                                          UINT(resViews.size()),
                                          resViews.data());
            context->PSSetConstantBuffers(1, 2, constBuffers);

            // Volume Rendering
            if (mesh->densityTex.GetSRV())
                context->PSSetShaderResources(
                    5, 1, mesh->densityTex.GetAddressOfSRV());
            if (mesh->lightingTex.GetSRV())
                context->PSSetShaderResources(
                    6, 1, mesh->lightingTex.GetAddressOfSRV());

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);
            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                                      DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed(mesh->indexCount, 0, 0);

            // Release resources
            ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
            context->PSSetShaderResources(5, 3, nulls);
        }
    }
}


void Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                            int frame) {
    // class SkinnedMeshModel���� override
    cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
            "int clipId, int frame) was not implemented."
         << endl;
    exit(-1);
}

void Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                            float frame) {
    cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
            "int clipId, int frame) was not implemented."
         << endl;
    exit(-1);
}

void Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                            int currClipId, int nextClipId, int frame) {
    UpdateAnimation(context, currClipId, frame);
}

void Model::RenderNormals(ComPtr<ID3D11DeviceContext> &context) {
    for (const auto &mesh : m_meshes) {
        ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                         mesh->materialConstsGPU.Get()};
        context->GSSetConstantBuffers(1, 2, constBuffers);
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &mesh->stride, &mesh->offset);
        context->Draw(mesh->vertexCount, 0);
    }
}

void Model::RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context) {
    ID3D11Buffer *constBuffers[2] = {
        m_boundingBoxMesh->meshConstsGPU.Get(),
        m_boundingBoxMesh->materialConstsGPU.Get()};
    context->VSSetConstantBuffers(1, 2, constBuffers);
    context->IASetVertexBuffers(
        0, 1, m_boundingBoxMesh->vertexBuffer.GetAddressOf(),
        &m_boundingBoxMesh->stride, &m_boundingBoxMesh->offset);
    context->IASetIndexBuffer(m_boundingBoxMesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_boundingBoxMesh->indexCount, 0, 0);
}

void Model::RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context) {
    ID3D11Buffer *constBuffers[2] = {
        m_boundingBoxMesh->meshConstsGPU.Get(),
        m_boundingBoxMesh->materialConstsGPU.Get()};
    context->VSSetConstantBuffers(1, 2, constBuffers);
    context->IASetVertexBuffers(
        0, 1, m_boundingSphereMesh->vertexBuffer.GetAddressOf(),
        &m_boundingSphereMesh->stride, &m_boundingSphereMesh->offset);
    context->IASetIndexBuffer(m_boundingSphereMesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_boundingSphereMesh->indexCount, 0, 0);
}

void Model::UpdateScale(Vector3 scale) { 
        m_scale = scale;
    UpdateWorldRow();
}

void Model::UpdatePosition(Vector3 position) { 
        m_position = position;
    UpdateWorldRow();
}

void Model::UpdateRotation(Vector3 rotation) { 
        m_rotation = rotation; 
        UpdateWorldRow();
}

void Model::UpdateTranseform(Vector3 scale, Vector3 rotation,
                             Vector3 position) {
    m_scale = scale;
    m_position = position;
    m_rotation = rotation;

    UpdateWorldRow();
}

void Model::AddYawOffset(float addYawOffset) 
{ 
        m_rotation.y += addYawOffset;
    UpdateWorldRow();
}

void Model::UpdateWorldRow() {

    m_worldRow = Matrix::CreateScale(m_scale.x) *
                 Matrix::CreateRotationX(m_rotation.x) *
                 Matrix::CreateRotationY(m_rotation.y) *
                 Matrix::CreateRotationZ(m_rotation.z) *
                 Matrix::CreateTranslation(m_position);

    m_worldITRow = m_worldRow;
    m_worldITRow.Translation(Vector3(0.0f));
    m_worldITRow = m_worldITRow.Invert().Transpose();

    // �ٿ�����Ǿ� ��ġ ������Ʈ
    // �����ϱ��� ����ϰ� �ʹٸ� x, y, z ������ �� ���� ū ������ ������
    // ��(sphere)�� ȸ���� ����� �ʿ� ����
    m_boundingSphere.Center = this->m_worldRow.Translation();

    m_meshConsts.GetCpu().world = m_worldRow.Transpose();
    m_meshConsts.GetCpu().worldIT = m_worldITRow.Transpose();
    m_meshConsts.GetCpu().worldInv = m_meshConsts.GetCpu().world.Invert();
}



} // namespace hlab