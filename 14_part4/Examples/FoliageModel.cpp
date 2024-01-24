#include "FoliageModel.h"
#include "BillboardModel.h"
#include "GraphicsPSO.h"
#include "AppBase.h"
#include "Camera.h"

#include <cstdlib>
#include <queue>

using namespace hlab;
using namespace std;

hlab::FoliageModel::FoliageModel(ComPtr<ID3D11Device> &device,
                                 ComPtr<ID3D11DeviceContext> &context,
                                 const vector<MeshData> &meshes,
                                 AppBase *appBase, vector<int> &meshStartID) {
    m_allowDuplicateMesh = true;
    m_appBase = appBase;
    Initialize(device, context, meshes);

    m_meshStartID = meshStartID;
    MakeBoundingBox(device, meshes); 
}
    
void hlab::FoliageModel::MakeBoundingBox(
    ComPtr<ID3D11Device> &device, const vector<MeshData> &meshDatas) {
         
    for (int meshID : m_meshStartID) {
        auto &mesh = meshDatas[meshID]; 
        Vector3 minCorner(10000.f);
        Vector3 maxCorner(0.f);
         
        for (const auto& vtx : mesh.vertices) {
            minCorner = Vector3::Min(minCorner, vtx.position);
            maxCorner = Vector3::Max(maxCorner, vtx.position);
        }
        Vector3 Center = (minCorner + maxCorner) * 0.5f;
        Vector3 Extents = maxCorner - Center; 
        m_boundingBoxs.push_back(BoundingBox(Center, Extents));
          
        shared_ptr<BillboardModel> tempBM =  
            make_shared<BillboardModel>(m_appBase);  
        Vector4 tempPos = Vector4(Center.x, Center.y, Center.z, 1.0);

        tempBM->Initialize(device, m_appBase->m_context, {tempPos}, 2,
                           Graphics::basicPS);
        tempBM->m_geometryShader = Graphics::foliageGS; 
        tempBM->resViews = 
        {m_meshes[0]->billboardDiffuseSRV.Get(),
                            m_meshes[0]->billboardNormalSRV.Get(), 
                            m_meshes[0]->aoSRV.Get(),
                            m_meshes[0]->metallicRoughnessSRV.Get(),
                            m_meshes[0]->emissiveSRV.Get(),
                            m_meshes[0]->billboardArtSRV.Get()}; 

        tempBM->useOtherShaderResource = true;
        tempBM->m_materialConsts.GetCpu().useAlbedoMap = true; 
        tempBM->m_materialConsts.GetCpu().useNormalMap = true;
        tempBM->m_materialConsts.Upload(m_appBase->m_context);
        tempBM->m_meshConsts.GetCpu().useARTTexture = 1;
         

        std::random_device rd;
        std::mt19937 gen(rd()); 
        std::uniform_int_distribution<int> dis(0, 6);

        ConstantBuffer<FoliageConsts> tempConsts;
        tempConsts.Initialize(device);
        tempConsts.GetCpu().foliageID = int(dis(gen));
        tempBM->m_foliageConstsGPU = tempConsts.Get();
        tempConsts.Upload(m_appBase->m_context); 
        
        m_billboards.push_back(tempBM);    
    } 
}   
      
void hlab::FoliageModel::MakeBVH() { 
         
        m_bvh.clear();
    if (m_boundingBoxs.size() == 0)
        return;
     
    auto CreateBVNode = [&](int min, int max) {
        BoundingBox enclosingBox = m_boundingBoxs[min];
        Vector3 box1center = enclosingBox.Center;
        Vector3 box1Extents = enclosingBox.Extents;

        vector<Vector3> box1Corners = {
            box1center + Vector3(-1.0f, -1.0f, -1.0f) * box1Extents,
            box1center + Vector3(-1.0f, 1.0f, -1.0f) * box1Extents,
            box1center + Vector3(1.0f, 1.0f, -1.0f) * box1Extents,
            box1center + Vector3(1.0f, -1.0f, -1.0f) * box1Extents,
            box1center + Vector3(-1.0f, -1.0f, 1.0f) * box1Extents,
            box1center + Vector3(-1.0f, 1.0f, 1.0f) * box1Extents,
            box1center + Vector3(1.0f, 1.0f, 1.0f) * box1Extents,
            box1center + Vector3(1.0f, -1.0f, 1.0f) * box1Extents};
        for (int i = 0; i < box1Corners.size(); i++) {
            box1Corners[i] = Vector3::Transform(box1Corners[i], m_worldRow);
        }


        Vector3 minCorner = box1Corners[0];
        Vector3 maxCorner = box1Corners[0];
        for (int i = 1; i < box1Corners.size(); i++) {
                minCorner = Vector3::Min(minCorner, box1Corners[i]);
                maxCorner = Vector3::Max(maxCorner, box1Corners[i]);
        }

        for (int i = min + 1; i < max; i++) {


                Vector3 box2center = m_boundingBoxs[i].Center;
                Vector3 box2Extents = m_boundingBoxs[i].Extents;

                vector<Vector3> box2Corners = {
                    box2center + Vector3(-1.0f, -1.0f, -1.0f) * box2Extents,
                    box2center + Vector3(-1.0f, 1.0f, -1.0f) * box2Extents,
                    box2center + Vector3(1.0f, 1.0f, -1.0f) * box2Extents,
                    box2center + Vector3(1.0f, -1.0f, -1.0f) * box2Extents,
                    box2center + Vector3(-1.0f, -1.0f, 1.0f) * box2Extents,
                    box2center + Vector3(-1.0f, 1.0f, 1.0f) * box2Extents,
                    box2center + Vector3(1.0f, 1.0f, 1.0f) * box2Extents,
                    box2center + Vector3(1.0f, -1.0f, 1.0f) * box2Extents };
                for (int i = 0; i < box2Corners.size(); i++) {
                        box2Corners[i] = Vector3::Transform(box2Corners[i], m_worldRow);
                }

                for (int j = 0; j < box2Corners.size(); j++) {
                        minCorner = Vector3::Min(minCorner, box2Corners[j]);
                        maxCorner = Vector3::Max(maxCorner, box2Corners[j]);
                }
        }
        enclosingBox.Center = (minCorner + maxCorner) * 0.5f;
        enclosingBox.Extents = (maxCorner - minCorner) * 0.5f;

        BVNode result = BVNode(enclosingBox);
        if (max - min == 1) {
                result.objectID = min;
        }
        return result;
    };

    std::queue<std::pair<int, int>> queue;
    queue.push(make_pair(0, m_boundingBoxs.size()));

    int index = 0;
    while (!queue.empty()) {
            int min = queue.front().first;
            int max = queue.front().second;
            queue.pop();
            m_bvh.push_back(CreateBVNode(min, max));

            int midValue = (max + min) / 2;
            int nextMin = min;
            int nextMax = midValue;
            if (nextMax - nextMin > 0 && nextMax != max) {
                    queue.push(make_pair(nextMin, nextMax));
                    m_bvh.back().leftChildID = ++index;
            }

            nextMin = midValue;
            nextMax = max;
            if (nextMax - nextMin > 0 && nextMin != min) {
                    queue.push(make_pair(nextMin, nextMax));
                    m_bvh.back().rightChildID = ++index; 
            }
    }
}
 
void hlab::FoliageModel::GetMeshInFrustum(
    bool isFindingReflectModel) {

        vector<shared_ptr<Mesh>>* resultModel;
        vector<shared_ptr<Mesh>>* resultDistantModel;
        vector<shared_ptr<Model>>* resultBillboardMesh;

        if (isFindingReflectModel == false) {
            resultModel = &m_foundMesh;
            resultDistantModel = &m_foundDistantMesh;
            resultBillboardMesh = &m_foundBillboardFoliages;
        } else { 
            resultModel = &m_foundReflectMesh;
            resultDistantModel = &m_foundDistantReflectMesh;
            resultBillboardMesh = &m_foundBillboardReflectFoliages;
        }

         
        resultModel->resize(0);
        resultDistantModel->resize(0);
        resultBillboardMesh->resize(0);
         
        //bool isMirror = renderState == ERenderState::reflect;
        bool isMirror = isFindingReflectModel;
        AppBase::MyFrustum frustum; 
        frustum.InitFrustum(m_appBase, isMirror);

        Vector3 cameraPos = m_appBase->m_camera->GetPosition();

        std::queue<pair<BVNode, int>> queue;
        
        if (m_bvh.size() == 0)
                return;
         
        static int id = 0; 
        queue.push(make_pair(m_bvh[0], id));
         
        while (!queue.empty()) {
                BVNode& node = queue.front().first; 
                int index = queue.front().second;
                queue.pop();

                Vector3 center = node.boundingBox.Center;
                Vector3 Extents = node.boundingBox.Extents;
                // Matrix t = m_worldRow;
                Matrix t;

                if (isMirror) {

                    center = 
                        Vector3::Transform(center, m_appBase->m_reflectRow);

                    Vector3 tempT = m_appBase->m_reflectRow.Translation();
                    m_appBase->m_reflectRow.Translation(Vector3());
                    Extents =
                        Vector3::Transform(Extents, m_appBase->m_reflectRow);
                     
                    m_appBase->m_reflectRow.Translation(tempT);
                }
                        
                bool check = frustum.Intersects(center, Extents, t); 

                if (check) {   
                           
                    if (node.objectID >= 0)  
                    {      
                        float cameraToCenter = (cameraPos - center).Length();

                        shared_ptr<Mesh> &tempMesh = 
                            m_meshes[m_meshStartID[node.objectID]]; 
                        int vertexId =min(0, int(cameraToCenter));  
                        vertexId =
                            min(int(tempMesh->vertexBuffers.size() )- 1, vertexId);
                        tempMesh->vertexBuffer =
                            tempMesh->vertexBuffers[vertexId];
                        tempMesh->vertexCount = tempMesh->vertexCounts[vertexId];
                         
                         if (cameraToCenter < billboardDistance){ 
                            if (cameraToCenter < shadowDistance)
                                    resultModel->push_back(tempMesh);
                            else  
                                resultDistantModel->push_back(tempMesh); 
                         } else { 
                                resultBillboardMesh->push_back( 
                                m_billboards[node.objectID]);
                         }
                    } 
                 
                 
                        int leftID = m_bvh[index].leftChildID;
                        int rightID = m_bvh[index].rightChildID;
                        if (leftID < m_bvh.size())
                        queue.push(make_pair(m_bvh[leftID], leftID));
                        if (rightID < m_bvh.size())
                        queue.push(make_pair(m_bvh[rightID], rightID));
                }
        }
}    
       
void hlab::FoliageModel::Render(ComPtr<ID3D11DeviceContext> &context) {
               
        if (isDestory || m_isVisible == false)
                return;

    static int searchingMeshTimer = 20; 
         
        if (searchingMeshTimer > 10) { 
                GetMeshInFrustum(false);
                GetMeshInFrustum(true);
                searchingMeshTimer = 0;
        } 
        searchingMeshTimer++;     
                 
         
        if (renderState != ERenderState::reflect) 
        {
                RenderFoliage(context, m_foundMesh);
                 
                if (m_appBase->isRenderShadowMap == false)
                        RenderFoliage(context, m_foundDistantMesh);

                for (auto &billboardFoliage : m_foundBillboardFoliages) {
                        billboardFoliage->Render(context);
                } 
                   
        } else if (m_appBase->m_foundMirror) {
                RenderFoliage(context, m_foundReflectMesh);
                if (m_appBase->isRenderShadowMap == false)
                        RenderFoliage(context, m_foundDistantReflectMesh);
                          
                for (auto &billboardFoliage : m_foundBillboardReflectFoliages) {
                        billboardFoliage->Render(context);
                } 
        } 

}  
 
void hlab::FoliageModel::RenderFoliage(ComPtr<ID3D11DeviceContext> &context,
                                       vector<shared_ptr<Mesh>> &meshes) { 
        if (meshes.size() == 0) { 
                return;     
        }  
         for (int i = 0; i < meshes.size(); i++){
                    
                shared_ptr<Mesh> mesh = meshes[i];
                ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                                 mesh->materialConstsGPU.Get()};
                context->VSSetConstantBuffers(1, 2, constBuffers);

                context->VSSetShaderResources(0, 1,
                                              mesh->heightSRV.GetAddressOf());

                // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
                vector<ID3D11ShaderResourceView *> resViews = {
                    mesh->albedoSRV.Get(),   mesh->normalSRV.Get(),
                    mesh->aoSRV.Get(),       mesh->metallicRoughnessSRV.Get(),
                    mesh->emissiveSRV.Get(), mesh->artSRV.Get()};
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

                vector<ID3D11Buffer *> vertexBuffers = {
                    mesh->vertexBuffer.Get()};
                context->IASetVertexBuffers(0, UINT(vertexBuffers.size()),
                                            vertexBuffers.data(), &mesh->stride,
                                            &mesh->offset);
                  
                context->Draw(mesh->vertexCount, 0);

                // Release resources
                ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
                context->PSSetShaderResources(5, 3, nulls);
    }
}   
  
void hlab::FoliageModel::UpdateWorldRow(Vector3 &scale, Vector3 &rotation,
                                        Vector3 &position) {
    Model::UpdateWorldRow(scale, rotation, position);
    for (auto& billboards : m_billboards){
                billboards->UpdateTranseform(scale, rotation, position - Vector3(0.0f, 0.1f, 0.0f)); 
                billboards->m_meshConsts.Upload(m_appBase->m_context);
            } 

    MakeBVH(); 
}
   