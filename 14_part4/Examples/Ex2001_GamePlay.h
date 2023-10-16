#pragma once

        
#include "AppBase.h"
#include "BillboardModel.h"
#include "physx/PxPhysicsAPI.h"
#include "SkinnedMeshModel.h"

// 기본 사용법은 SnippetHelloWorld.cpp
// 렌더링 관련은 SnippetHelloWorldRender.cpp
#include <directxtk/audio.h>

#define PX_RELEASE(x)                                                          \
    if (x) {                                                                   \
        x->release();                                                          \
        x = NULL;                                                              \
    }

#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 128

namespace hlab {

using namespace physx;

class Ex2001_GamePlay : public AppBase {
  public:
    Ex2001_GamePlay();

    ~Ex2001_GamePlay() {
        PX_RELEASE(gScene);
        PX_RELEASE(gDispatcher);
        PX_RELEASE(gPhysics);
        if (gPvd) {
            PxPvdTransport *transport = gPvd->getTransport();
            gPvd->release();
            gPvd = NULL;
            PX_RELEASE(transport);
        }
        PX_RELEASE(gFoundation);
    }

    bool InitScene() override;

    PxRigidDynamic *CreateDynamic(const PxTransform &t,
                                  const PxGeometry &geometry,
                                  const PxVec3 &velocity = PxVec3(0));
    void CreateStack(const PxTransform &t, int numStacks, int numSlices,
                     PxReal halfExtent);
    void InitPhysics(bool interactive);
    void UpdateLights(float dt) override;
    void UpdateGUI() override;
    void Update(float dt) override;
    void UpdateAnim(float dt);
    void Render() override;
    void InitAudio();

    double GetTimeSeconds() { return timeSeconds; };

  public:
    float m_simToRenderScale = 0.01f; // 시뮬레이션 물체가 너무 작으면 불안정

    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;
    PxFoundation *gFoundation = NULL;
    PxPhysics *gPhysics = NULL;
    PxDefaultCpuDispatcher *gDispatcher = NULL;
    PxScene *gScene = NULL;
    PxMaterial *gMaterial = NULL;
    PxPvd *gPvd = NULL;
    PxReal stackZ = 10.0f;

    // 물리 엔진과 동기화 시켜줄 때 사용 TODO: actor list로 변경
    vector<shared_ptr<Model>> m_PhysicalObjects; 
    shared_ptr<Model> m_ocean;

    shared_ptr<BillboardModel> m_fireball;
    shared_ptr<class Character> m_player;

    bool isUsingSkill = false;
    bool bUseBlendAnimation = true;

    std::unique_ptr<DirectX::AudioEngine> m_audEngine;
    std::unique_ptr<DirectX::SoundEffect> m_sound;
    double timeSeconds = 0.0;

    float metallicFactor = 0.0f;
    float roughnessFactor = 0.0f;
    int useAlbedoMap = 0.0f;
    int useEmissiveMap = 0.0f;
    int useNormalMap = 0.0f;

    int useAOMap = 0.0f;
    int useHeightMap = 0.0f;
    float heightScale = 0.0f;
    int useMetallicMap = 0.0f;
    int useRoughnessMap = 0.0f;
    bool drawNormals = 0.0f;

};

} // namespace hlab
