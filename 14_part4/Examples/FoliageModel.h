#pragma once

#include "Model.h"
#include "AppBase.h"
  
namespace hlab {
class FoliageModel : public Model{
  public:
    FoliageModel(){};
    FoliageModel(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context,
                 const vector<MeshData> &meshes, class AppBase *appBase,
                 vector<int> &meshStartID);
      
    void MakeBoundingBox(
        ComPtr<ID3D11Device> &device, const vector<MeshData> &meshDatas);
    void MakeBVH();
    void GetMeshInFrustum();
    virtual void Render(ComPtr<ID3D11DeviceContext> &context) override;
    void RenderFoliage(ComPtr<ID3D11DeviceContext> &context,
                       vector<shared_ptr<Mesh>> &meshes);
    float billboardDistance = 10.0f;
    float shadowDistance = 1.0f;
       
private:
    vector<ComPtr<ID3D11Buffer>> m_vertexBuffers;
    vector<BoundingBox> m_boundingBoxs;
    vector<int> m_meshStartID;
    vector<BVNode> m_bvh;  
    vector<shared_ptr<Mesh>> m_foundMesh; 
    vector<shared_ptr<Mesh>> m_foundDistantMesh; 
    shared_ptr<Mesh> m_mergeMesh; 
    shared_ptr<Mesh> m_mergeDistantMesh;
    vector<pair<shared_ptr<Mesh>, ComPtr<ID3D11Buffer>>> m_foundBillboardMesh; 
     
    

        bool isBBDRendering = false;
        virtual void UpdateWorldRow(Vector3 &scale, Vector3 &rotation,
                                Vector3 &position)override;
};
}
 