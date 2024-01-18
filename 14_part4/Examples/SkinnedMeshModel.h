#pragma once

#include "GeometryGenerator.h"
#include "Model.h"
#include <iostream>

#include "AppBase.h"
#include <filesystem>
#include <DirectXMath.h>
#include <queue>
#include <cmath>

namespace hlab {
        using namespace std;
using namespace DirectX;
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

    virtual void InitMeshBuffers(ComPtr<ID3D11Device> &device, const MeshData &meshData, shared_ptr<Mesh> &newMesh) override{
                 vector<SkinnedVertex> initVtx;
                vector<seed_seq::result_type> initIdx;
        {  
                map<Vector3, int> uniVtx;
                 vector<pair<int, int>> indexs(meshData.skinnedVertices.size());
                  
                 for (int i = 0; i < meshData.skinnedVertices.size(); i++) {
                        auto &vtx = meshData.skinnedVertices[i];
                        auto it = uniVtx.find(vtx.position);
                        if (it == uniVtx.end()) {
                            initVtx.push_back(vtx);
                            uniVtx.insert(make_pair(vtx.position, i)); 

                            indexs[i].first = i;
                            indexs[i].second = indexs[max(0, i - 1)].second;
                        } else {
                            //newVtx.push_back(
                            //    meshData.skinnedVertices[it->second]); 
                                indexs[i].first = it->second; 
                            indexs[i].second = indexs[max(0, i - 1)].second + 1; 
                        }
                } 
                 for (int i = 0; i < meshData.indices.size(); i++) {
                        int id = meshData.indices[i]; 
                        id = indexs[id].first - indexs[indexs[id].first].second; 
                        if (id >= initVtx.size())
                            continue;
                        initIdx.push_back(
                            seed_seq::result_type(id));
                 }  
                 
                 D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                                                newMesh->vertexBuffer);
                 newMesh->vertexBuffers.push_back(newMesh->vertexBuffer);
                 
                 D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                               newMesh->indexBuffer); 
                 newMesh->indexBuffers.push_back(newMesh->indexBuffer);
                  
                 newMesh->indexCount = UINT(meshData.indices.size());
                 newMesh->indexCounts.push_back(newMesh->indexCount);
                 newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
                 newMesh->vertexCounts.push_back(newMesh->vertexCount);
                  
          

                newMesh->stride = UINT(sizeof(SkinnedVertex));
                 
        }
        
        static const int minVertexCount = 3;
         
        vector<vector<SkinnedVertex>> newVertexs;
        vector<vector<seed_seq::result_type>> newIndexs;
        newVertexs.push_back(initVtx);
        newIndexs.push_back(initIdx); 
        
        int lodCount = 5;
        while (lodCount--) { 
                vector<SkinnedVertex> nextVtx;
                vector<seed_seq::result_type> nextIdx;

                vector<pair<int, int>> newIdInfo(newVertexs.back().size(), make_pair(-1, 0));

                for (int i = 0; i < newIndexs.back().size(); i+= 3) {
                       vector<int> edge;
                       {
                            int v1 = newIndexs.back()[i];
                            int v2 = newIndexs.back()[min(int(newIndexs.back().size()) - 1, i + 1)];
                            int v3 = newIndexs.back()[min(int(newIndexs.back().size()) - 1, i + 2)];
                                     
                            if (newIdInfo[v1].first == -1) 
                                edge.push_back(v1); 
                            if (v1 != v2 && newIdInfo[v2].first == -1)
                                edge.push_back(v2);    
                            if (edge.size() < 2 && v2 != v3 && v1 != v3 &&
                                newIdInfo[v3].first == -1) 
                                edge.push_back(v3);
                               
                            if (edge.size() == 2 &&  edge [0] > edge[1]) 
                            {
                                int temp = edge[0];
                                edge[0] = edge[1];
                                edge[1] = temp;
                            }
                       }
                         
                       if (edge.size() < 2)
                            continue;

                       newIdInfo[edge[0]].first = edge[0];
                       newIdInfo[edge[1]].first = edge[0];

                }
                for (int i = 0; i < newIdInfo.size(); i++) {
                       if (newIdInfo[i].first == -1)
                            newIdInfo[i].first = i; 
                        
                       if (newIdInfo[i].first == i)
                            newIdInfo[i].second =
                                newIdInfo[max(i - 1, 0)].second;
                       else {
                            newIdInfo[i].second =
                                newIdInfo[max(i - 1, 0)].second + 1;
                       }
                }
                    
                for (int i = 0; i < newVertexs.back().size(); i++) {
                       if (newIdInfo[i].first == i) {  
                                nextVtx.push_back(newVertexs.back()[i]);
                       } 
                       else
                       {
                                int baseId = newIdInfo[i].first;
                                 
                                if (baseId - newIdInfo[baseId].second >=
                                    nextVtx.size()) 
                                cout << "baseId : " << baseId
                                     << ", newIdInfo[baseId].second :"
                                     << newIdInfo[baseId].second << "\n";
                                 
                                SkinnedVertex &baseVtx =
                                        nextVtx[baseId - newIdInfo[baseId].second];
                                baseVtx = SkinnedVertex::InterporlationVertex(
                                        baseVtx, newVertexs.back()[i]); 
                       }
                } 
                for (int j = 0; j < newIndexs.back().size() - 2; j += 3) {
                             
                        int i1 = newIndexs.back()[j];
                       int i2 = newIndexs.back()[j + 1]; 
                        int i3 = newIndexs.back()[j + 2];  
                         
                        i1 = newIdInfo[i1].first;
                        i2 = newIdInfo[i2].first;
                        i3 = newIdInfo[i3].first;
                            
                        if (i1 == i2 || i1 == i3 || i2 == i3) {
                            continue;
                        }

                        nextIdx.push_back(i1 - newIdInfo[i1].second);
                        nextIdx.push_back(i2 - newIdInfo[i2].second);
                        nextIdx.push_back(i3 - newIdInfo[i3].second); 
                } 
                 
                newVertexs.push_back(nextVtx);
                newIndexs.push_back(nextIdx); 
                nextVtx.clear();
                nextIdx.clear();
                if (newIndexs.back().size() > 3) {

                        D3D11Utils::CreateVertexBuffer(device, newVertexs.back(),
                                                       newMesh->vertexBuffer);
                        newMesh->vertexBuffers.push_back(newMesh->vertexBuffer);
                        newMesh->vertexCounts.push_back(newVertexs.back().size());

                         
                        D3D11Utils::CreateIndexBuffer(device, newIndexs.back(),
                                                      newMesh->indexBuffer);
                        newMesh->indexBuffers.push_back(newMesh->indexBuffer);
                        newMesh->indexCounts.push_back(newIndexs.back().size());
                } else 
                        break; 
                   
        } 
           
        int idx = newMesh->vertexBuffers.size() - 1;
        idx = 0;
        newMesh->vertexBuffer =
            newMesh->vertexBuffers[idx];
        newMesh->indexBuffer = newMesh->indexBuffers[idx];
        newMesh->indexCount = newMesh->indexCounts[idx];
        newMesh->vertexCount = newMesh->vertexCounts[idx];
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
    void ChangeAnimation(EActorState& animID, bool forward);

    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         float frame) override;

    void Render(ComPtr<ID3D11DeviceContext> &context) override {


        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의
        for (auto& mesh : m_meshes)
        {
            mesh->vertexBuffer =
                mesh->vertexBuffers[min(int(mesh->vertexBuffers.size() )- 1, m_appBase->renderingLod)];
            mesh->indexBuffer = mesh->indexBuffers[min(
                int(mesh->indexBuffers.size()) - 1, m_appBase->renderingLod)];
            mesh->indexCount = mesh->indexCounts[min(
                int(mesh->indexCounts.size()) - 1, m_appBase->renderingLod)];
            mesh->vertexCount = mesh->vertexCounts[min(
                int(mesh->vertexCounts.size()) - 1, m_appBase->renderingLod)];
        }
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
    bool isFalling = false;

};

} // namespace hlab