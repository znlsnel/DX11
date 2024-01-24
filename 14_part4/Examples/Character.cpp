#include "Character.h"
#include "SkinnedMeshModel.h"
#include "Ex2001_GamePlay.h"

using namespace hlab;
hlab::Character::Character(AppBase* base, ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const string meshPath, const string meshFileName,
        const vector<string> clipNames) {
        auto [meshes, _] =
                GeometryGenerator::ReadAnimationFromFile(meshPath, meshFileName);

        AnimationData aniData;
        for (auto& name : clipNames) {
                auto [_, ani] =
                        GeometryGenerator::ReadAnimationFromFile(meshPath, name);

                if (aniData.clips.empty()) {
                        aniData = ani;
                }
                else {
                        aniData.clips.push_back(ani.clips.front());
                }
        }
        Vector3 center(0.0f, 0.1f, 1.0f);

        m_mesh =
                make_shared<SkinnedMeshModel>(device, context, meshes, aniData);

        m_mesh->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        m_mesh->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
        m_mesh->m_materialConsts.GetCpu().metallicFactor = 0.0f;

        //m_mesh->UpdateWorldRow(Matrix::CreateScale(0.2f) *
        //                            Matrix::CreateTranslation(center));
        m_mesh->UpdateTranseform(Vector3(0.2f), m_mesh->GetRotation(), center);
        m_mesh->SetLocalPosition(Vector3(0.0f, 0.1f, 0.0f));
        m_appBase = (Ex2001_GamePlay*)base;

}

void hlab::Character::Update(float dt) {
        UpdateState(dt);
        UpdateTransform(dt);
}

void hlab::Character::BeginPlay() {

}


void hlab::Character::UpdateTransform(float dt)
{


        if (m_appBase->m_camera->m_objectTargetCameraMode == false)
                return;
        if (m_appBase->m_keyPressed['A']) {
                m_mesh->AddYawOffset(-3.141592f * 240.f / 180.f * dt);
        }
        else if (m_appBase->m_keyPressed['D']) {
                m_mesh->AddYawOffset(3.141592f * 240.f / 180.f * dt);
        }

        Vector3 vc;
        if (state == walk || state == run) {
                Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
                dir = Vector4::Transform(
                        dir, m_mesh->m_worldRow);

                dir.Normalize();
                float speed = m_walkSpeed;
                if (state == run)
                        speed = m_runSpeed;

                Vector3 tdir = Vector3(dir.x, dir.y, dir.z);

                if (m_appBase->m_keyPressed['S'])
                        tdir *= -0.7f; 

                vc = tdir * speed  * 0.3f;
                Vector3 velocity = m_mesh->m_worldRow.Translation() + vc * dt;

                m_mesh->UpdatePosition(velocity);
        }

         
        static Vector3 dir = Vector3(0.0f, -1.0f, 0.0f);
        static Vector3 velocity;

        bool isJumping =
            m_mesh->isFalling == false && m_appBase->m_keyPressed[VK_SPACE];

        //vc.Normalize();
        if (isJumping) { 
                state = EActorState::jump;
                m_mesh->isFalling = true; 
                m_appBase->m_keyPressed[VK_SPACE] = false;

                velocity = vc + Vector3(0.0f, 0.5f, 0.0f);
        }

        Vector3 pos = m_mesh->GetPosition();
        Vector3 origin = pos + Vector3(0.0f, 0.5f, 0.0f);
        float dist = 0.0f;
        m_appBase->SetHeightPosition(origin, dir, dist);
         
        if (dist > 0.0f)
        {
                if (pos.y >  origin.y - dist) 
                { 
                        Vector3 temp;
                        Vector3 nextPos = pos + velocity * dt;
                        Vector3 groundPos = origin + Vector3(0.0f, -dist + 0.1f, 0.0f);
                        if (nextPos.y > groundPos.y)
                        {
                            velocity += dir * dt;
                        } 
                        else 
                        {
                            nextPos = groundPos;
                            velocity = Vector3(0.0f, 0.0f, 0.0f);
                        }

                        //  점프 시작 위치 높으면 falling
                        // 바닥에 닿으면 ㄴㄴ falling
                        m_mesh->isFalling = nextPos.y - groundPos.y > 0.025f;
                                            
                        m_mesh->UpdatePosition(nextPos);         
                }
        }
         
}

void hlab::Character::UpdateState(float dt) {

        if (m_appBase->m_camera->m_objectTargetCameraMode == false)
                return;

        switch (state) { 
        case EActorState::idle:
        {
                  if (m_appBase->m_keyPressed['W'] ||
                             m_appBase->m_keyPressed['S'])
                        state = EActorState::walk;
        }
                break;
        case EActorState::walk: {
                  if (m_appBase->m_keyPressed['W'] == false &&
                      m_appBase->m_keyPressed['S'] == false)
                        state = EActorState::idle;
                  else if (m_appBase->m_keyPressed[VK_SHIFT])
                  {
                        state = EActorState::run;
                  } 
        }
                break;
        case EActorState::run: {
                  if (m_appBase->m_keyPressed['W'] == false &&
                      m_appBase->m_keyPressed['S'] == false)
                        state = EActorState::idle;
                   
                  else if (m_appBase->m_keyPressed[VK_SHIFT] == false) {
                        state = EActorState::walk;
                  }
        }
                break;
        case EActorState::jump: {
                  // if (m_mesh->currAnim == &m_mesh->jumping_Down) {
                  //       float rt = m_mesh->jumping_Down.frame /
                  //                  (float)m_mesh->jumping_Down.endFrame;
                  //         if (rt > 0.8f)
                  //               state = EActorState::idle;
                  // }
        } break;
        } 
        
        m_mesh->ChangeAnimation(state, m_appBase->m_keyPressed['W']);
          m_mesh->UpdatePose(m_appBase->m_context, dt);
   //    m_mesh->UpdateAnimation(appBase->m_context, 0, 0);

}

int hlab::Character::GetCurrAnimSize() 
{ 
        return (int)(m_mesh->m_aniData.clips[state].keys[0].size());
}

