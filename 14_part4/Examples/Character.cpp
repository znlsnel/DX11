#include "Character.h"
#include "SkinnedMeshModel.h"
#include "Ex2001_GamePlay.h"

using namespace hlab;
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
        } else if (m_appBase->m_keyPressed['D']) {
                m_mesh->AddYawOffset(3.141592f * 240.f / 180.f * dt);
        } 

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

                  Vector3 velocity = m_mesh->m_worldRow.Translation() +
                                     tdir * speed * dt * 0.3f;

                   
                  m_mesh->UpdatePosition(velocity);
        }

        static float updateTime = 0.0f;
        static const float updateCycle = 1.0f / 165.0f;

        if (updateTime > updateCycle) 
        {
                Vector3 pos = m_mesh->GetPosition();
                  Vector3 origin = pos + Vector3(0.0f, 0.5f, 0.0f);
                float dist = 0.0f;
                m_appBase->SetHeightPosition(origin, Vector3(0.0f, -1.0f, 0.0f), dist);

                if (dist > 0.0f)
                        m_mesh->UpdatePosition(origin + (Vector3(0.0f, -1.0f, 0.0f) * dist) + Vector3(0.0f, 0.1f, 0.0f));
                updateTime = 0.0f;        
        }
        updateTime += dt;
}

void hlab::Character::UpdateState(float dt) {

        if (m_appBase->m_camera->m_objectTargetCameraMode == false)
                return;

        switch (state) { 
        case EActorState::idle:
        {
                  if (m_appBase->m_keyPressed[VK_SPACE]) {
                        //state = EActorState::attack;
                  } 
                  else if (m_appBase->m_keyPressed['W'] ||
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
        }
        
          //std::cout << "state : " << state << std::endl;
        m_mesh->ChangeAnimation(state, m_appBase->m_keyPressed['W']);
          m_mesh->UpdatePose(m_appBase->m_context, dt);
   //    m_mesh->UpdateAnimation(appBase->m_context, 0, 0);

}

int hlab::Character::GetCurrAnimSize() 
{ 
        return (int)(m_mesh->m_aniData.clips[state].keys[0].size());
}

