#pragma once
#include "GeometryGenerator.h"
#include "SkinnedMeshModel.h"
#include "Model.h"

namespace hlab{




        using namespace std;

class Character {
  public:
    Character(class AppBase* base, ComPtr<ID3D11Device> &device,
              ComPtr<ID3D11DeviceContext> &context, const string meshPath,
              const string meshFileName, const vector<string> clipNames);

    virtual void Update(float dt);
    virtual void BeginPlay();
    virtual void UpdateState(float dt);
    virtual void UpdateTransform(float dt);

    void SetUseUpdateTick(bool use) { bUseUpdateTick = use; };
    int GetCurrAnimSize();

    shared_ptr<SkinnedMeshModel> GetMesh() { return m_mesh; };

    float m_simToRenderScale = 0.01f;
     
    private:
       class Ex2001_GamePlay* m_appBase;
        shared_ptr<SkinnedMeshModel> m_mesh;
         
        bool bUseUpdateTick = false;
        bool isUsingSkill = false;
          
         
        EActorState state = EActorState::idle;
         
        float m_walkSpeed = 1.0f;
        float m_runSpeed = 3.0f;
};
}

    