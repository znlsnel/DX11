#include "Character.h"
#include "SkinnedMeshModel.h"
#include "Ex2001_GamePlay.h"

hlab::Character::Character(AppBase* base, ComPtr<ID3D11Device> &device,
                           ComPtr<ID3D11DeviceContext> &context,
                           const string meshPath, const string meshFileName,
                           const vector<string> clipNames) {
        auto [meshes, _] =
                GeometryGenerator::ReadAnimationFromFile(meshPath, meshFileName);

        AnimationData aniData;
        for (auto &name : clipNames) {
                auto [_, ani] =
                GeometryGenerator::ReadAnimationFromFile(meshPath, name);

                if (aniData.clips.empty()) {
                aniData = ani;
                } else {
                aniData.clips.push_back(ani.clips.front());
                } 
        }
        Vector3 center(0.0f, 0.1f, 1.0f);

        m_mesh =
                make_shared<SkinnedMeshModel>(device, context, meshes, aniData);

        m_mesh->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        m_mesh->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
        m_mesh->m_materialConsts.GetCpu().metallicFactor = 0.0f;
        m_mesh->UpdateWorldRow(Matrix::CreateScale(0.2f) *
                                    Matrix::CreateTranslation(center));
        appBase = (Ex2001_GamePlay*)base;

}

void hlab::Character::Update(float dt) { 
        
        UpdateAnimation(dt); 

}

void hlab::Character::BeginPlay() {

}

void hlab::Character::UpdateAnimation(float dt) {
        int animSize = GetCurrAnimSize();
        // States
        // 0: idle
        // 1: idle to walk
        // 2: walk forward
        // 3: walk to stop
        // 4: dance

        bool isLastFrame = animSize <= frameCount;
        // /*animSize - frameCount < 20 || */(float)frameCount / (float)animSize
        // > 0.9;

        Vector3 tempPos = (m_mesh->m_worldRow).Translation();

        if (appBase->m_keyPressed[VK_SPACE] || state == 4) {
                if (state != 4) {
                state = 4;
                isLastFrame = true;
                //m_sound->Play();
                } else {
                if (isLastFrame)
                    state = 0;

                if ((int)frameCount == 60 && isUsingSkill == false) {
                    Vector3 handPos = (m_mesh->m_worldRow).Translation();
                    Vector4 offset = Vector4::Transform(
                        Vector4(0.0f, 0.0f, -0.1f, 0.0f),
                        m_mesh->m_worldRow *
                            m_mesh->m_aniData.accumulatedRootTransform);

                    // Vector3 offset = Vector3(0.0f, 0.0f, -0.1f);
                    handPos += Vector3(offset.x, offset.y, offset.z);

                    Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
                    dir = Vector4::Transform(
                        dir,
                        m_mesh->m_worldRow *
                                 m_mesh->m_aniData.accumulatedRootTransform);

                    dir.Normalize();
                    dir *= 1.5f / m_simToRenderScale;

                    appBase->CreateDynamic(
                        PxTransform(PxVec3(handPos.x, handPos.y, handPos.z) /
                                    m_simToRenderScale),
                        PxSphereGeometry(5), PxVec3(dir.x, dir.y, dir.z));
                    isUsingSkill = true;
                } else if ((int)frameCount != 60)
                    isUsingSkill = false;
                }
        }

        else if (state == 0) { // 정지 상태
                // TODO:
                if (appBase->m_keyPressed['W']) {
                state = 1;
                isLastFrame = true;
                }
        } else if (state == 1) {
                if (appBase->m_keyPressed['W']) {
                if (isLastFrame)
                    state = 2;
                } else {
                state = 3;
                isLastFrame = true;
                }
        }

        else if (state == 2) {
                if (appBase->m_keyPressed['W'] == false) {
                state = 3;
                isLastFrame = true;
                }
        }

        else if (state == 3) {
                if (appBase->m_keyPressed['W']) {
                state = 1;
                isLastFrame = true;
                } else if (isLastFrame) {
                state = 0;
                }
        }
        if (state != 4) {
                //if (appBase->m_keyPressed['D']) {
                //m_mesh->m_aniData.accumulatedRootTransform =
                //    Matrix::CreateRotationY(3.141592f * 120.0f / 180.0f * dt) *
                //    m_mesh->m_aniData.accumulatedRootTransform;
                //}
                //if (appBase->m_keyPressed['A']) {
                //m_mesh->m_aniData.accumulatedRootTransform =
                //    Matrix::CreateRotationY(-3.141592f * 120.0f / 180.0f * dt) *
                //    m_mesh->m_aniData.accumulatedRootTransform;
                //}
        }

        if (state > 0 && state < 4) {
                //Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
                //dir = Vector4::Transform(
                //    dir, m_mesh->m_worldRow *
                //             m_mesh->m_aniData.accumulatedRootTransform);

                //dir.Normalize();

                //Vector3 velocity = m_mesh->m_worldRow.Translation() +
                //                   (Vector3(dir.x, dir.y, dir.z)) * dt / 5.0f;
                //m_mesh->UpdateWorldRow(
                //    Matrix::CreateScale(0.2f) *
                //        // m_mesh->m_worldRow
                //        //     .CreateRotationY(appBase->m_camera.GetYaw()) *
                //        Matrix::CreateRotationY(appBase->m_camera.GetYaw()) *
                //        Matrix::CreateRotationY(3.141592f) *
                //    m_mesh->m_worldRow.CreateTranslation(velocity));
        }

        if (isLastFrame)
                frameCount = 0;

        m_mesh->UpdateAnimation(appBase->m_context, state, frameCount);

        frameCount += dt * 60;
}

int hlab::Character::GetCurrAnimSize() 
{ 
        return (int)(m_mesh->m_aniData.clips[state].keys[0].size());
}

