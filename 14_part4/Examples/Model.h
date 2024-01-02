#pragma once

#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"
#include "Mesh.h"
#include "MeshData.h"
#include "StructuredBuffer.h"

#include <directxtk/SimpleMath.h>
#include <DirectXCollision.h>
// 참고: DirectX-Graphics-Sampels
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

namespace hlab {

using std::cout;
using std::endl;
using std::string;
using std::vector;
using DirectX::SimpleMath::Vector3;
using namespace DirectX;

enum ERenderState : int {
        basic = 1,
        depth = 2,
        reflect = 3,
};

struct BoundingCollision {
  public:
    BoundingCollision() { 
            m_bb = BoundingBox(); 
    };
    BoundingCollision(Vector3 center, Vector3 extents) {
        m_bb = BoundingBox(center, extents);
    }
    bool Intersects(Vector3 origin, Vector3 dir, float &dist) {
        return m_bb.Intersects(origin, dir, dist);
    };
    bool TriangleIntersects(Vector3 origin, Vector3 dir, float& dist) {

        bool result = false;
        if (worldVertexs.size() < 3)
                return result;

        DirectX::SimpleMath::Ray ray = SimpleMath::Ray(origin, dir);
        result = ray.Intersects(worldVertexs[0], worldVertexs[1],
                worldVertexs[2], dist);
        if (result == false && worldVertexs.size() > 5)
                result = ray.Intersects(worldVertexs[3], worldVertexs[4],
                                        worldVertexs[5], dist);
         
        // 0 1 2 3  - 4  
        for (int i = 0; i <= worldVertexs.size() - 3; i++) {
                result = ray.Intersects(worldVertexs[i], worldVertexs[i+1],
                                        worldVertexs[i+2], dist);
                if (result == false && i + 1 < worldVertexs.size()) {
                    result = ray.Intersects(worldVertexs[i+2], worldVertexs[i + 1],
                                       worldVertexs[i + 3], dist);
                }
                if (result) 
                    break;
        }

        return result;
    }

    void updateWorldVertex(Matrix worldRow) { 

            worldVertexs.resize(vertexs.size());
        for (int i = 0; i < vertexs.size(); i++) {
                worldVertexs[i] = Vector3::Transform(vertexs[i], worldRow);
        }
    }
    BoundingBox m_bb;
    vector<Vector3> vertexs;
    vector<Vector3> worldVertexs;
};





class Model {
  public: 

    Model(){};
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,const string &basePath, const string &filename, class AppBase* appBase = nullptr);
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const vector<MeshData> &meshes, class AppBase *appBase = nullptr);

    virtual void Initialize(ComPtr<ID3D11Device> &device,
                            ComPtr<ID3D11DeviceContext> &context);

    virtual void InitMeshBuffers(ComPtr<ID3D11Device> &device,
                                 const MeshData &meshData,
                                 shared_ptr<Mesh> &newMesh);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const string &basePath, const string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes);
    virtual void UpdatePose(ComPtr<ID3D11DeviceContext> &context, float dt
                            ){};
    
    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    virtual GraphicsPSO &GetPSO(const bool wired);
    virtual GraphicsPSO &GetDepthOnlyPSO();
    virtual GraphicsPSO &GetReflectPSO(const bool wired);

    virtual void Render(ComPtr<ID3D11DeviceContext> &context);
    virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                                 int clipId, int frame);
    virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                                 int clipId, float frame);
    virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                                 int currClipId, int nextClipId, int frame);

    virtual void RenderNormals(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderBVH(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context);
    
    void UpdateScale(Vector3 scale);
    void UpdatePosition(Vector3 position);
    void SetLocalPosition(Vector3 pos) { m_localPosition = pos; };
    void UpdateRotation(Vector3 ratation);
    void UpdateTranseform(Vector3 scale, Vector3 rotation, Vector3 position);
    void AddYawOffset(float addYawOffset);
    void DestroyObject();

    Vector3 GetPosition() { return m_position; };
    Vector3 GetRotation() { return m_rotation; };
    Vector3 GetScale() { return m_scale; };

    static void ExtractEulerAnglesFromQuaternion(const SimpleMath::Quaternion& q,
            Vector3& angle) {
        angle.x = std::asin(2.0f * (q.x * q.z + q.y * q.w));

        //yaw = atan2(2 * (q1q2 + q0q3), q0 ^ 2 + q1 ^ 2 - q2 ^ 2 - q3 ^ 2)
        angle.y = std::atan2(2.0f * (q.y * q.z + q.x * q.w),
                             std::pow(q.x, 2.0f) + std::pow(q.y, 2.0f) -
                                 std::pow(q.z, 2.0f) - std::pow(q.w, 2.0f));
          
        //roll = atan2(2 * (q0q1 + q2q3), q0 ^ 2 - q1 ^ 2 - q2 ^ 2 + q3 ^ 2)
        angle.z = std::atan2(2.0f * (q.x * q.y + q.z * q.w),
                             std::pow(q.x, 2.0f) - std::pow(q.y, 2.0f) -
                                 std::pow(q.z, 2.0f) + std::pow(q.w, 2.0f)); 
    }

    static void ExtractEulerAnglesFromMatrix(const Matrix *worldRow, Vector3 &angle) {
        ExtractRollFromMatrix(worldRow, angle.z); 
        ExtractYawFromMatrix(worldRow, angle.y); 
        ExtractPitchFromMatrix(worldRow, angle.x);
    };
    static void ExtractPositionFromMatrix(const Matrix *worldRow, Vector3 &pos){
        pos.x = worldRow->_41;
        pos.y = worldRow->_42;
        pos.z = worldRow->_43;
    };
    static void ExtractScaleFromMatrix(const Matrix *worldRow,
                                          Vector3 &scale) {
        scale.x = worldRow->_11;
        scale.y = worldRow->_22; 
        scale.z = worldRow->_33;
    };
    static void ExtractRollFromMatrix(const Matrix *worldRow, float &roll) {
        //roll = std::atan2(worldRow->_12, worldRow->_11);
        roll = std::atan2(worldRow->_32, worldRow->_33);
    }; 
    static void ExtractYawFromMatrix(const Matrix *worldRow, float &Yaw) {
        //Yaw = std::atan2(worldRow->_23, worldRow->_33);
        Yaw = std::atan2(worldRow->_21, worldRow->_11);
    };
    static void ExtractPitchFromMatrix(const Matrix *worldRow, float &pitch) {
        //pitch = std::atan2(-worldRow->_13,
        //                   std::sqrt(worldRow->_23 * worldRow->_23 +
        //                             worldRow->_33 * worldRow->_33));
        pitch = std::atan2(-worldRow->_31, 
                           std::sqrt(worldRow->_32 * worldRow->_32 +
                                     worldRow->_33 * worldRow->_33));
    };

    void SetChildModel(shared_ptr<Model> model);
    void SetBVH(ComPtr<ID3D11Device> device, vector<BoundingCollision> &bvhBoxs,
                vector<shared_ptr<Mesh>>& bvhMeshs,
            const MeshData &mesh, int minIndex,
                int maxIndex, int level);
  protected:
    virtual void UpdateWorldRow(Vector3& scale, Vector3& rotation, Vector3& position);

  public:
    void UpdateWorldRow(const Matrix& row, bool debug = false);
         

    Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true; 
    bool m_isPickable = false; // 마우스로 선택/조작 가능 여부
    bool m_saveable = false;
    bool m_editable = false;
    bool m_drawBackFace = false; 
     int m_BVHMaxLevel = 0;
    bool isChildModel = false; 
    bool isObjectLock = false;
    bool bRenderingBVH = false;
    bool isDestory = false;
    bool isCursorShpere = false;

    int maxRenderingBVHLevel = 0; 
    int tempInt = 0;
      
    vector<shared_ptr<Mesh>> m_meshes;

    ConstantBuffer<MeshConstants> m_meshConsts;
    ConstantBuffer<MaterialConstants> m_materialConsts;
    DirectX::BoundingBox m_boundingBox;
    DirectX::BoundingSphere m_boundingSphere;
    vector<vector<BoundingCollision>> m_BVHs;
    ERenderState renderState = ERenderState::basic;
    //                        [0]
    //          [1]                        [2]
    //    [3]       [4]           [5]           [6]
    // [7] [8] [9] [10] [11] [12] [13] [14] 
    // 왼쪽 자식 :  *2 + 1
    // 오른쪽 자식 :  *2 + 2



    string m_name = "NoName";
    ObjectSaveInfo objectInfo;
    vector<shared_ptr<Model>> childModels;

  protected:
    vector<vector<shared_ptr<Mesh>>> m_BVHMesh;

    shared_ptr<Mesh> m_boundingBoxMesh;
    shared_ptr<Mesh> m_boundingSphereMesh;

    Vector3 m_scale{1.f}; 
    Vector3 m_position{0.f};
    Vector3 m_localPosition{0.f};
    Vector3 m_rotation{0.f};
    float m_boundingSphereRadius = 0.0f;
    class AppBase *m_appBase = nullptr; 


};

} // namespace hlab
