#pragma once

#include "GeometryGenerator.h"
#include "Model.h"

namespace hlab {

using std::make_shared;

class SkinnedMeshModel : public Model {
  public:
    SkinnedMeshModel(ComPtr<ID3D11Device> &device,
                     ComPtr<ID3D11DeviceContext> &context,
                     const vector<MeshData> &meshes,
                     const AnimationData &aniData) {
        Initialize(device, context, meshes, aniData);
    }

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes,
                    const AnimationData &aniData) {
        InitAnimationData(device, aniData);
        Model::Initialize(device, context, meshes);
    }

    GraphicsPSO &GetPSO(const bool wired) override {
        return wired ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
    }

    GraphicsPSO &GetReflectPSO(const bool wired) override {
        return wired ? Graphics::reflectSkinnedWirePSO
                     : Graphics::reflectSkinnedSolidPSO;
    }

    GraphicsPSO &GetDepthOnlyPSO() override {
        return Graphics::depthOnlySkinnedPSO;
    }

    void InitMeshBuffers(ComPtr<ID3D11Device> &device, const MeshData &meshData,
                         shared_ptr<Mesh> &newMesh) override {
        D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                                       newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
        newMesh->stride = UINT(sizeof(SkinnedVertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      newMesh->indexBuffer);
    }

    void InitAnimationData(ComPtr<ID3D11Device> &device,
                           const AnimationData &aniData) {
        if (!aniData.clips.empty()) {
            m_aniData = aniData;

            // ���⼭�� AnimationClip�� SkinnedMesh��� �����ϰڽ��ϴ�.
            // �Ϲ������δ� ��� Animation�� SkinnedMesh Animation�� �ƴմϴ�.
            m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

            // ����: ��� keys() ������ �������� ���� ���� �ֽ��ϴ�.
            for (int i = 0; i < aniData.clips.front().keys.size(); i++)
                m_boneTransforms.m_cpu[i] = Matrix();
            m_boneTransforms.Initialize(device);
        }
    }

    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         int frame) override {

        m_aniData.Update(clipId, frame);

        for (int i = 0; i < m_boneTransforms.m_cpu.size(); i++) {
            m_boneTransforms.m_cpu[i] =
                m_aniData.Get(clipId, i, frame).Transpose();
        }

        m_boneTransforms.Upload(context);
    }

    void Render(ComPtr<ID3D11DeviceContext> &context) override {

        // ConstBuffer ��� StructuredBuffer ���
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // �׻� slot index ����

        // Skinned VS/PS�� GetPSO()�� ���ؼ� �����Ǳ� ������
        // Model::Render(.)�� ���� ��� ����

        Model::Render(context);
    };

    // SkinnedMesh�� BoundingBox�� �׸� �� Root�� Transform�� �ݿ��ؾ� �մϴ�.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);

  public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;

    AnimationData m_aniData;
};

} // namespace hlab