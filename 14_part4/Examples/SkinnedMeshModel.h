#pragma once

#include "GeometryGenerator.h"
#include "Model.h"
#include <iostream>


namespace hlab {

        enum EActorState : int
        {
                idle = 0,
                walk = 1,
                run = 2,
                jump = 3,
        };

static bool isValid(void *ptr) { return ptr != nullptr; };

using std::make_shared;


class SkinnedMeshModel : public Model {
    struct Animation {
      public:
        Animation(){};
        Animation(int animID, size_t maxFrame, AnimationData *animData, Animation* preAnim = nullptr, Animation* nextAnim = nullptr) {
            clipId = animID;
            endFrame = (int)maxFrame;
            aniData = animData;

            startAnim = preAnim;
            endAnim = nextAnim;
        }

        void InitFrame()
        {
            frame = 0.0f;
            lerpValue = 0.0f;
            if (startAnim != nullptr)
                startAnim->InitFrame();

                return;
        };

        void GetCurrFramePos(vector<Matrix>& posMat, float dt, bool useBlendFrame = false) {

            float *tempFrame = &frame;
                if (useBlendFrame)
                tempFrame = &blendFrame;


                int currFrame = (int)*tempFrame; 
            int nextFrame = std::min(currFrame + 1, endFrame);
                float lerpValue = (float)*tempFrame - currFrame;

            aniData->Update(clipId, currFrame);
            for (int i = 0; i < posMat.size(); i++) {
                posMat[i] = aniData->Get(clipId, i, currFrame).Transpose();
            }

            aniData->Update(clipId, nextFrame);
            for (int i = 0; i < posMat.size(); i++) {

                posMat[i] = Matrix::Lerp(
                    posMat[i], aniData->Get(clipId, i, nextFrame).Transpose(),
                    lerpValue);
            }
            
               *tempFrame += (dt * 60.f);


            //std::cout << "frame : " << frame << std::endl;

            if ((int)*tempFrame > endFrame) {
               *tempFrame = 0.0f;
            }

        };

        Animation* GetStartAnim(){ 

                if (startAnim == nullptr)
                        return startAnim;

                Animation* temp = startAnim->startAnim;
                if (temp == nullptr)
                        return startAnim;
                else
                        return startAnim->GetStartAnim();
        };

        void SetStartAnim(Animation* anim) { 
                Animation *temp = GetStartAnim();

                if (temp == nullptr) {
                        startAnim = anim;
                        anim->endAnim = this;                
                } else {
                        temp->startAnim = anim;
                        anim->endAnim = temp;
                }
        }


        bool isPlaying() { 
                return (int)frame < endFrame;

        };
        bool isPlayingStartAnim() {
                if (startAnim == nullptr)
                        return false;

                if (startAnim->isPlaying())
                        return true;

                return startAnim->isPlayingStartAnim();
        };


        int clipId = 0;

        float frame = 0;
        float blendFrame = 0;
        int startFrame = 0;
        int endFrame = 0;
        float blendTime = 1.0f;
        float lerpValue = 0.0f;

        AnimationData* aniData = nullptr;
        Animation *startAnim = nullptr;
        Animation *endAnim = nullptr;
    };

  public:
    SkinnedMeshModel(ComPtr<ID3D11Device> &device,
                     ComPtr<ID3D11DeviceContext> &context,
                     const vector<MeshData> &meshes,
                     const AnimationData &aniData) {
        Initialize(device, context, meshes, aniData);
    }



    void UpdatePose(ComPtr<ID3D11DeviceContext> &context, float dt)override;
    

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes,
                    const AnimationData &aniData) {
        InitAnimationData(device, aniData);
        Model::Initialize(device, context, meshes);
        AnimationInit();
    }
    void AnimationInit();

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

            // 여기서는 AnimationClip이 SkinnedMesh라고 가정하겠습니다.
            // 일반적으로는 모든 Animation이 SkinnedMesh Animation은 아닙니다.
            m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

            // 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
            for (int i = 0; i < aniData.clips.front().keys.size(); i++)
                m_boneTransforms.m_cpu[i] = Matrix();
            m_boneTransforms.Initialize(device);
        }
    }
    void ChangeAnimation(Animation *anim); 
    void ChangeAnimation(EActorState animID, bool forward);

    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         float frame) override;

    void Render(ComPtr<ID3D11DeviceContext> &context) override {


        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의

        Model::Render(context);
    };

    bool isPlaying(Animation *anim) {
        if (anim == nullptr)
            return false;

        return anim->isPlaying();
    };

    // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);

  public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;

    AnimationData m_aniData;

    Animation idle;
    Animation walk;
    Animation walk_backward;
    Animation run;
    Animation run_backward;
    Animation jumping_Up;
    Animation jumping_Down;
    Animation jumping_Falling;
    Animation turn_Left;
    Animation turn_Right;

    Animation *prevAnim = nullptr;
    Animation *currAnim = &idle;
    Animation *nextAnim = nullptr;

    int currAnimID = 0;
    int currAnimFrame = 0;
    int currAnimSize = 0;
    float lerpRatio = 0.0f;

    float blendingTime = 0.0f;
};

} // namespace hlab