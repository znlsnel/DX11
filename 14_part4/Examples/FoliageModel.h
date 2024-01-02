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
     
    void MakeBoundingBox(const vector<MeshData>& meshDatas);
    void MakeBVH();
    void GetMeshInFrustum(vector<shared_ptr<Mesh>> &mesh);
    virtual void Render(ComPtr<ID3D11DeviceContext> &context) override;
    //virtual GraphicsPSO &GetPSO(const bool wired);
    //virtual GraphicsPSO &GetDepthOnlyPSO();
    //virtual GraphicsPSO &GetReflectPSO(const bool wired);
    // 
       
    vector<BoundingBox> m_boundingBoxs;
    vector<int> m_meshStartID;
    vector<BVNode> m_bvh; 
    vector<shared_ptr<Mesh>> m_foundMesh; 
     
    private:
        bool isBBDRendering = false;
      virtual void UpdateWorldRow(Vector3 &scale, Vector3 &rotation,
                                  Vector3 &position)override;
};
}
 