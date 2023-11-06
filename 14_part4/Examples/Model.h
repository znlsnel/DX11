﻿#pragma once

#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"
#include "Mesh.h"
#include "MeshData.h"
#include "StructuredBuffer.h"

#include <directxtk/SimpleMath.h>

// 참고: DirectX-Graphics-Sampels
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

namespace hlab {

using std::cout;
using std::endl;
using std::string;
using std::vector;

enum ERenderState : int {
        basic = 1,
        depth = 2,
        reflect = 3,
};


class Model {
  public:
    Model(){};
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const string &basePath, const string &filename);
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const vector<MeshData> &meshes);

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
    virtual void UpdatePose(ComPtr<ID3D11DeviceContext> &context, float dt,
                            bool bUseBlendAnim = false){};
    
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
    void UpdateRotation(Vector3 ratation);
    void UpdateTranseform(Vector3 scale, Vector3 rotation, Vector3 position);
    void AddYawOffset(float addYawOffset);

    Vector3 GetPosition() { return m_position; };
    Vector3 GetRotation() { return m_rotation; };
    Vector3 GetScale() { return m_scale; };

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
        roll = std::atan2(worldRow->_12, worldRow->_11);
    };
    static void ExtractYawFromMatrix(const Matrix *worldRow, float &Yaw) {
        Yaw = std::atan2(worldRow->_23, worldRow->_33);
    };
    static void ExtractPitchFromMatrix(const Matrix *worldRow, float &pitch) {
        pitch = std::atan2(-worldRow->_13,
                           std::sqrt(worldRow->_23 * worldRow->_23 +
                                     worldRow->_33 * worldRow->_33));
    };

    void SetChildModel(shared_ptr<Model> model);
    void SetBVH(ComPtr<ID3D11Device> device,
                vector<DirectX::BoundingBox>& bvhBoxs,
                vector<shared_ptr<Mesh>>& bvhMeshs,
            const MeshData &mesh, int minIndex,
                int maxIndex, int level);
  private:
    void UpdateWorldRow(Vector3& scale, Vector3& rotation, Vector3& position);

public:
         

    Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true;
    bool m_isPickable = false; // 마우스로 선택/조작 가능 여부
    bool isDestory = false;
    bool isChildModel = false;
    bool isObjectLock = false;
    bool bRenderingBVH = false;
    bool m_saveable = false;
    bool m_editable = false;
    bool isPlane = false;

    int maxRenderingBVHLevel = 0;


    vector<shared_ptr<Mesh>> m_meshes;

    ConstantBuffer<MeshConstants> m_meshConsts;
    ConstantBuffer<MaterialConstants> m_materialConsts;
    DirectX::BoundingBox m_boundingBox;
    DirectX::BoundingSphere m_boundingSphere;
    vector<vector<DirectX::BoundingBox>> m_BVHs;
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
    Vector3 m_rotation{0.f};
    float m_boundingSphereRadius = 0.0f;
};

} // namespace hlab
