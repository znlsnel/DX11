#pragma once

        
#include "AppBase.h"
#include "BillboardModel.h"
#include "physx/PxPhysicsAPI.h"
#include "SkinnedMeshModel.h"

// �⺻ ������ SnippetHelloWorld.cpp
// ������ ������ SnippetHelloWorldRender.cpp
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
  public:
    float m_simToRenderScale = 0.01f; // �ùķ��̼� ��ü�� �ʹ� ������ �Ҿ���

    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;
    PxFoundation *gFoundation = NULL;
    PxPhysics *gPhysics = NULL;
    PxDefaultCpuDispatcher *gDispatcher = NULL;
    PxScene *gScene = NULL;
    PxMaterial *gMaterial = NULL;
    PxPvd *gPvd = NULL;
    PxReal stackZ = 10.0f;

    vector<shared_ptr<Model>>
        m_objects; // ���� ������ ����ȭ ������ �� ��� TODO: actor list�� ����

    shared_ptr<BillboardModel> m_fireball;
    shared_ptr<class Character> m_player;

    bool isUsingSkill = false;

    std::unique_ptr<DirectX::AudioEngine> m_audEngine;
    std::unique_ptr<DirectX::SoundEffect> m_sound;
};

} // namespace hlab
