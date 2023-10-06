#pragma once

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




class Model {
  public:
    Model() {}
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
    virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context);
    
    void UpdateScale(Vector3 scale);
    void UpdatePosition(Vector3 position);
    void UpdateRotation(Vector3 ratation);
    void UpdateTranseform(Vector3 scale, Vector3 rotation, Vector3 position);
    void AddYawOffset(float addYawOffset);

    Vector3 GetPosition() { return m_position; };
    Vector3 GetRotation() { return m_rotation; };
    Vector3 GetScale() { return m_scale; };

    static void ExtractEulerAnglesFromMatrix(const Matrix *worldRow, float &yaw,
                                      float &roll, float &pitch) {
        ExtractRollFromMatrix(worldRow, roll);
        ExtractYawFromMatrix(worldRow, yaw);
        ExtractPitchFromMatrix(worldRow, pitch);
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
  private:
    void UpdateWorldRow();

  public:


    Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true;
    bool m_isPickable = false; // 마우스로 선택/조작 가능 여부
    bool useTessellation = false;

    vector<shared_ptr<Mesh>> m_meshes;

    ConstantBuffer<MeshConstants> m_meshConsts;
    ConstantBuffer<MaterialConstants> m_materialConsts;

    DirectX::BoundingBox m_boundingBox;
    DirectX::BoundingSphere m_boundingSphere;

    string m_name = "NoName";
    ObjectSaveInfo objectInfo;


  private:
    shared_ptr<Mesh> m_boundingBoxMesh;
    shared_ptr<Mesh> m_boundingSphereMesh;

    Vector3 m_scale{1.f};
    Vector3 m_position{0.f};
    Vector3 m_rotation{0.f};
};

} // namespace hlab
