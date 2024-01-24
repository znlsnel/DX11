#pragma once

#include "Model.h"
#include "AppBase.h"
#include "BillboardModel.h"
  
namespace hlab {

struct FoliageConsts {
    int foliageID = 0; 
    int dummy[3] = {0, 0,0 };
};

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
    void GetMeshInFrustum(bool isFindingReflectModel); 


    virtual void Render(ComPtr<ID3D11DeviceContext> &context) override;
    void RenderFoliage(ComPtr<ID3D11DeviceContext> &context,
                       vector<shared_ptr<Mesh>> &meshes);
    float billboardDistance = 1.0f;
    float shadowDistance = 1.0f;
       
private:
    vector<shared_ptr<Model>> m_billboards;
    vector<BoundingBox> m_boundingBoxs;
    
    vector<shared_ptr<Mesh>> m_foundMesh; 
    vector<shared_ptr<Mesh>> m_foundReflectMesh;
    vector<shared_ptr<Mesh>> m_foundDistantMesh; 
    vector<shared_ptr<Mesh>> m_foundDistantReflectMesh; 

    vector<shared_ptr<Model>> m_foundBillboardFoliages;
    vector<shared_ptr<Model>> m_foundBillboardReflectFoliages;
    vector<int> m_meshStartID;
    vector<BVNode> m_bvh;  
     
     bool isBBDRendering = false;
     virtual void UpdateWorldRow(Vector3 &scale, Vector3 &rotation,
                                Vector3 &position)override;
};
}
  