﻿#pragma once

#include <directxtk/SimpleMath.h>

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

class Camera {
  public:
    Camera(class AppBase *appBase);

    Matrix GetViewRow();
    Matrix GetProjRow();
    Vector3 GetEyePos();
    
    void Reset(Vector3 pos, float yaw, float pitch) {
        m_position = pos;
        m_yaw = yaw;
        m_pitch = pitch;
        UpdateViewDir();
    }

    void UpdatePos();
    void UpdateViewDir();
    void UpdateKeyboard(const float dt, bool const keyPressed[256]);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetAspectRatio(float aspect);
    void SetLocation(Vector3 pos);
    void PrintView();
    void SetTarget(class Character *target) { m_target = target; };

    Vector3 GetForwardVector();
    Vector3 GetPosision();
    float GetYaw() { return m_yaw; };
  public:
    bool m_useFirstPersonView = false;
    bool m_isCameraLock = false;
    float cameraDistance = 1.0f;
    float cameraSpeed = 1.0f;

  private:
    Vector3 m_position = Vector3(0.312183f, 0.957463f, -1.88458f);
    Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_forwardDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f); // +Y 방향으로 고정
    Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float m_yaw = -0.0589047f, m_pitch = 0.213803f;

    float m_speed = 3.0f; // 움직이는 속도

    // 프로젝션 옵션도 카메라 클래스로 이동
    float m_projFovAngleY = 90.0f * 0.5f; // Luna 교재 기본 설정
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;

    bool m_usePerspectiveProjection = true;

    class Character* m_target;
    class AppBase *m_appBase;
};

} // namespace hlab
