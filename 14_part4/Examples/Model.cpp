
#include "Model.h"
#include "GeometryGenerator.h"
#include <filesystem>
#include <queue>

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



BoundingBox GetBoundingBox(const vector<hlab::Vertex> &vertices, int min, int max) {

    if (vertices.size() == 0)
        return BoundingBox();

    Vector3 minCorner = vertices[min].position;
    Vector3 maxCorner = vertices[min].position;


    for (size_t i = min + 1; i < max; i++) {
        minCorner = Vector3::Min(minCorner, vertices[i].position);
        maxCorner = Vector3::Max(maxCorner, vertices[i].position);
    }

    Vector3 center = (minCorner + maxCorner) * 0.5f;
    Vector3 extents = maxCorner - center;

    return BoundingBox(center, extents);
}
BoundingBox GetBoundingBox(const vector<hlab::Vertex> &vertices, const vector<uint32_t>& indices, int min, int max) {

    if (vertices.size() == 0)
        return BoundingBox();

    Vector3 minCorner = vertices[indices[min]].position;
    Vector3 maxCorner = vertices[indices[min]].position;

    for (size_t i = min + 1; i < max; i++) {
        minCorner = Vector3::Min(minCorner, vertices[indices[i]].position);
        maxCorner = Vector3::Max(maxCorner, vertices[indices[i]].position);
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

    // 일반적으로는 Mesh들이 m_mesh/materialConsts를 각자 소유 가능
    // 여기서는 한 Model 안의 여러 Mesh들이 Consts를 모두 공유

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

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
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

    // 
    m_BVHs.resize(meshes.size());
    m_BVHMesh.resize(meshes.size());
    // Initialize Main Bounding Box

    {
        m_boundingBox = 
            GetBoundingBox(meshes[0].vertices, 0, meshes[0].vertices.size());
        for (size_t i = 1; i < meshes.size(); i++) {

            auto bb =
                GetBoundingBox(meshes[i].vertices, 0, meshes[i].vertices.size());
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

    for (int i = 0; i < meshes.size(); i++) {
        SetBVH(device, m_BVHs[i], m_BVHMesh[i], meshes[i], 0,
               meshes[i].indices.size(), 0);
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
        maxRadius += 1e-2f; // 살짝 크게 설정
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
}
 
void Model::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) { 
            objectInfo.meshID = objectInfo.meshID;
      

        m_meshConsts.Upload(context);
        m_materialConsts.Upload(context);
    }
}

GraphicsPSO &Model::GetPSO(const bool wired) {

       renderState = ERenderState::basic;
    return wired ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO;
}

GraphicsPSO &Model::GetDepthOnlyPSO() { 
  
       renderState = ERenderState::depth;
    return Graphics::depthOnlyPSO;
}

GraphicsPSO &Model::GetReflectPSO(const bool wired) {
        
       renderState = ERenderState::reflect;
    return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
} 
 
 
void Model::Render(ComPtr<ID3D11DeviceContext> &context) {
          

    if (m_isVisible) { 
        for (const auto &mesh : m_meshes) {

            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            context->VSSetConstantBuffers(1, 2, constBuffers);
             
            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());

            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
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
    // class SkinnedMeshModel에서 override
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


    ID3D11Buffer *constBuffers[2] = {m_boundingBoxMesh->meshConstsGPU.Get(),
        m_boundingBoxMesh->materialConstsGPU.Get()};
    context->VSSetConstantBuffers(1, 2, constBuffers);
    context->IASetVertexBuffers(0, 1,
                                m_boundingBoxMesh->vertexBuffer.GetAddressOf(),
        &m_boundingBoxMesh->stride, &m_boundingBoxMesh->offset);
    context->IASetIndexBuffer(m_boundingBoxMesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_boundingBoxMesh->indexCount, 0, 0);
}

void Model::RenderBVH(ComPtr<ID3D11DeviceContext> &context) 
{
    int startIndex = 0;
    int maxIndex = 0;
    for (int i = 0; i < maxRenderingBVHLevel - 1; i++) {
        maxIndex = maxIndex * 2 + 2;
        //if (i == maxRenderingBVHLevel - 3)
        //    startIndex = maxIndex;
    }
//    cout << "maxIndex : " << maxIndex << endl;

    for (auto mesh : m_BVHMesh) {
        for (int i = startIndex; i < mesh.size(); i++) {
            
                    if (i >= maxIndex)
                        break;

               // cout << "rendering BVH level : " << i << "\n";
                ID3D11Buffer *constBuffers[2] = {
                mesh[i]->meshConstsGPU.Get(),
                mesh[i]->materialConstsGPU.Get()};

                context->VSSetConstantBuffers(1, 2, constBuffers);

                context->IASetVertexBuffers(
                    0, 1, mesh[i]->vertexBuffer.GetAddressOf(),
                    &mesh[i]->stride, &mesh[i]->offset);
                context->IASetIndexBuffer(mesh[i]->indexBuffer.Get(),
                                          DXGI_FORMAT_R32_UINT, 0);
                context->DrawIndexed(mesh[i]->indexCount, 0, 0);
            }
    }

}

void Model::RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context) {
    if (m_boundingBoxMesh == nullptr || m_boundingSphereMesh == nullptr)
            return;


    ID3D11Buffer *constBuffers[2] = {m_boundingBoxMesh->meshConstsGPU.Get(),
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
     //   m_scale = scale;
    UpdateWorldRow(scale, m_rotation, m_position);
}

void Model::UpdatePosition(Vector3 position) { 
  //      m_position = position;
    UpdateWorldRow(m_scale, m_rotation, position);
}

void Model::UpdateRotation(Vector3 rotation) { 
     //   m_rotation = rotation; 
        UpdateWorldRow(m_scale, rotation, m_position);
}

void Model::UpdateTranseform(Vector3 scale, Vector3 rotation,
                             Vector3 position) {


    UpdateWorldRow(scale, rotation, position);
}
  
void Model::AddYawOffset(float addYawOffset) 
{ 
        Vector3 tempRotation = m_rotation;
    tempRotation.y += addYawOffset;
    UpdateWorldRow(m_scale, tempRotation, m_position); 
}

void Model::SetChildModel(shared_ptr<Model> model) {
    if (model->isChildModel == false) {
        model->isChildModel;
    }
    childModels.push_back(model);
}

void Model::SetBVH(ComPtr<ID3D11Device> device,
                   vector<DirectX::BoundingBox> &BVHBoxs,
                   vector<shared_ptr<Mesh>> &BVBMeshs, 
                   const MeshData &mesh,
                   int minIndex, int maxIndex, int level) {

        std::queue < std::tuple<int, int, int >> queue;
    queue.push(std::make_tuple(minIndex, maxIndex, level));

    while (!queue.empty()) {
        auto curr = queue.front();
        queue.pop();

        int minID = std::get<0>(curr);
        int maxID = std::get<1>(curr);
        int currLevel = std::get<2>(curr);

        BVHBoxs.push_back(GetBoundingBox(mesh.vertices, mesh.indices, minID, maxID));
        auto meshData = GeometryGenerator::MakeWireBox(
            BVHBoxs.back().Center,
            Vector3(BVHBoxs.back().Extents) + Vector3(1e-3f));

        BVBMeshs.push_back(std::make_shared<Mesh>());

        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       BVBMeshs.back()->vertexBuffer);
        BVBMeshs.back()->indexCount = UINT(meshData.indices.size());
        BVBMeshs.back()->vertexCount = UINT(meshData.vertices.size());
        BVBMeshs.back()->stride = UINT(sizeof(Vertex));

        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      BVBMeshs.back()->indexBuffer);

        BVBMeshs.back()->meshConstsGPU = m_meshConsts.Get();
        BVBMeshs.back()->materialConstsGPU = m_materialConsts.Get();

        if (currLevel > 17)
            break;

        DirectX::BoundingBox& currBox = BVHBoxs.back();
        int InterID = (maxID + minID) / 2;

        int nextMinID = minID;
        int nextMaxID = InterID;

        if (nextMinID < nextMaxID - 4)
            queue.push(std::make_tuple(nextMinID, nextMaxID, currLevel + 1));


        nextMinID = nextMaxID;
        nextMaxID = maxID;

        if (nextMinID < nextMaxID - 4)
                queue.push(std::make_tuple(nextMinID, nextMaxID, currLevel + 1));

    }
    


    

}

void Model::UpdateWorldRow(Vector3 &scale, Vector3 &rotation,
                           Vector3 &position) {

     if (isObjectLock)
        return;

     m_scale = scale;
     m_rotation = rotation;
     m_position = position;

    m_worldRow = Matrix::CreateScale(m_scale.x, m_scale.y, m_scale.z) *
                 Matrix::CreateRotationX(m_rotation.x) *
                 Matrix::CreateRotationY(m_rotation.y) *
                 Matrix::CreateRotationZ(m_rotation.z) *
                 Matrix::CreateTranslation(m_position);

    m_worldITRow = m_worldRow;
    m_worldITRow.Translation(Vector3(0.0f));
    m_worldITRow = m_worldITRow.Invert().Transpose();

    // 바운딩스피어 위치 업데이트
    // 스케일까지 고려하고 싶다면 x, y, z 스케일 중 가장 큰 값으로 스케일
    // 구(sphere)라서 회전은 고려할 필요 없음
    m_boundingSphere.Center = this->m_worldRow.Translation();
    m_boundingSphere.Radius =
        m_boundingSphereRadius * max(m_scale.x, max(m_scale.y, m_scale.z));
    

    /*m_boundingSphereMesh*/
    m_meshConsts.GetCpu().world = m_worldRow.Transpose();
    m_meshConsts.GetCpu().worldIT = m_worldITRow.Transpose();
    m_meshConsts.GetCpu().worldInv = m_meshConsts.GetCpu().world.Invert();

    for (auto model : childModels) {
        model->UpdateTranseform(m_scale, m_rotation, m_position);
    }
}

} // namespace hlab