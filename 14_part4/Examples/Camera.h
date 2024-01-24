#pragma once

#include <directxtk/SimpleMath.h>
#include <memory>

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

class Camera {
  public:
    Camera(class AppBase *appBase);

    Matrix GetViewRow();
    Matrix GetCharacterViewRow();
    Matrix GetProjRow(bool frustumProj = false);
    Matrix GetShadowProjRow(Vector2 aspect, float farZ);
    Vector3 GetEyePos(); 
    
    void Reset(Vector3 pos, float yaw, float pitch) {
        m_position = pos;
        m_yaw = yaw;
        m_pitch = pitch;
        UpdateViewDir();
    }

    Vector3 GetCameraPosition();
    void UpdatePos();
    void UpdateViewDir();
    void UpdateKeyboard(const float dt, bool const keyPressed[256]);
    void RotateCamera(float addYaw, float addPitch);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetAspectRatio(float aspect);
    void SetLocation(Vector3 pos);
    void PrintView();
    void SetTarget(std::shared_ptr<class Character> target) { m_target = target; };
    Vector3 NdcToWorld(Vector3 ndc);
    Vector3 GetNdcDir(Vector2 ndc);

    std::shared_ptr<class Character> GetTarget() { return m_target; };
    Vector3 GetForwardVector();
    Vector3 GetRightVector() { return m_rightDir; };
    Vector3 GetPosition();
    float GetYaw() { return m_yaw; };
  public:
    bool m_objectTargetCameraMode = false;
    bool m_isCameraLock = true;
    bool m_UsingCharacterView = false;
    float cameraDistance = 1.0f;
    float cameraSpeed = 1.0f;
      
    float m_projFrustumAspect = 21.f / 9.f;
    float m_projFrustumAngleY = 90.0f * 0.5f; 

  private:
    Vector3 m_position = Vector3(0.312183f, 0.957463f, -1.88458f);
    Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_forwardDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f); // +Y 방향으로 고정
    Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 m_moveDir = Vector3(0.0f, 0.0f, 0.0f);
    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float m_yaw = -0.0589047f, m_pitch = 0.213803f;

    float m_speed = 3.0f; // 움직이는 속도

    // 프로젝션 옵션도 카메라 클래스로 이동
    float m_nearZ = 0.01f;
    float m_farZ = 300.0f; 
    float m_aspect = 16.0f / 9.0f;
    float m_projFovAngleY = 90.0f * 0.5f; // Luna 교재 기본 설정

   std::shared_ptr< class Character> m_target;
    class AppBase *m_appBase;  
};
 
} // namespace hlab
 