#include "Camera.h"
#include "Character.h"
#include "AppBase.h"

#include <iostream>

namespace hlab {

using namespace std;
using namespace DirectX;

Camera::Camera(AppBase* appBase) {
        UpdateViewDir(); 
        m_appBase = appBase;
}

Matrix Camera::GetViewRow() {
    return Matrix::CreateTranslation(-m_position) *
           Matrix::CreateRotationY(-m_yaw) *
           Matrix::CreateRotationX(-m_pitch); 

    
    // m_pitch가 양수이면 고개를 드는 방향 
}  
          
Matrix Camera::GetCharacterViewRow() {  
         
        Vector4 tempDir = Vector4::Transform(Vector4(0.0f, 0.0f, -1.0f, 0.0f),
                                     GetTarget()->GetMesh()->m_worldRow);  
        Vector3 dir = Vector3(tempDir.x, tempDir.y, tempDir.z);
        Vector3 pos = GetTarget()->GetMesh()->GetPosition()  - dir * 0.5f;
         
        return Matrix::CreateTranslation(-pos) *
           Matrix::CreateRotationY(-GetTarget()->GetMesh()->GetRotation().y + 3.141592) *
           Matrix::CreateRotationX(-GetTarget()->GetMesh()->GetRotation().x);
           Matrix::CreateRotationZ(-GetTarget()->GetMesh()->GetRotation().z);
} 

Vector3 Camera::GetEyePos() { return m_position; }
Vector3 Camera::GetCameraPosition() { 
        if (/*GetTarget() == nullptr || */m_UsingCharacterView == false) {
               return GetPosition();   
        }
        return GetTarget()->GetMesh()->GetPosition();
                   
}
void Camera::UpdatePos() {

        if (m_target != nullptr && m_objectTargetCameraMode) {
                Vector3 tempPos = m_target->GetMesh()->m_worldRow.Translation();
                //
                tempPos += -GetForwardVector() * cameraDistance;
                SetLocation(tempPos);
        } 
        else if (m_isCameraLock == false)
        {
                Vector3 moveDir{0.f, 0.f, 0.f};
                
                if (m_appBase->m_keyPressed['W'])
                    moveDir += m_forwardDir;

                if (m_appBase->m_keyPressed['S'])
                    moveDir += -m_forwardDir;
                    
                if (m_appBase->m_keyPressed['A']) {
                    moveDir += -m_rightDir;
                }

                if (m_appBase->m_keyPressed['D'])
                    moveDir += m_rightDir;

                if (m_appBase->m_keyPressed['Q'])
                    moveDir += -m_upDir;

                if (m_appBase->m_keyPressed['E'])
                    moveDir += m_upDir;

                moveDir.Normalize();

                if (moveDir.Length() == 0.f)
                    m_moveDir = m_moveDir * 0.97f;
                else
                        m_moveDir = moveDir;
        }
        

        if (m_moveDir.Length() > 0.02f)
                SetLocation(m_position + m_moveDir * 0.1 * cameraSpeed);
        m_moveDir *= 0.97f;
}

void Camera::UpdateViewDir() {
    // 이동할 때 기준이 되는 정면/오른쪽 방향 계산
    m_viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                          Matrix::CreateRotationY(this->m_yaw));
    m_forwardDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationX(this->m_pitch) *
                                       Matrix::CreateRotationY(this->m_yaw));
    m_forwardDir.Normalize();


    m_rightDir = m_upDir.Cross(m_viewDir);
}

void Camera::UpdateKeyboard(const float dt, bool const keyPressed[256]) {
    //if (m_useFirstPersonView) {
    //    if (keyPressed['W'])
    //        MoveForward(dt);
    //    if (keyPressed['S'])
    //        MoveForward(-dt);
    //    if (keyPressed['D'])
    //        MoveRight(dt);
    //    if (keyPressed['A'])
    //        MoveRight(-dt);
    //    if (keyPressed['E'])
    //        MoveUp(dt);
    //    if (keyPressed['Q'])
    //        MoveUp(-dt);
    //}
}

void Camera::RotateCamera(float addYaw, float addPitch) {
        // 얼마나 회전할지 계산
    if (m_isCameraLock)
                return;

        m_yaw += addYaw * DirectX::XM_2PI;         // 좌우 360도
        m_pitch += addPitch * DirectX::XM_PIDIV2; // 위 아래 90도

        if (m_yaw > DirectX::XM_2PI)
                m_yaw = -DirectX::XM_2PI;
        if (m_yaw < -DirectX::XM_2PI)
                m_yaw = DirectX::XM_2PI;

        //if (m_yaw > DirectX::XM_2PI)
        //        m_yaw = -DirectX::XM_2PI;
        //if (m_yaw < -DirectX::XM_2PI)
        //        m_yaw = DirectX::XM_2PI;

        m_pitch = std::clamp(m_pitch, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);
        UpdateViewDir();
    
}

void Camera::MoveForward(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveUp(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_upDir * m_speed * dt;
}

void Camera::MoveRight(float dt) { m_position += m_rightDir * m_speed * dt; }

void Camera::SetAspectRatio(float aspect) { m_aspect = aspect; }

void Camera::SetLocation(Vector3 pos) { m_position = pos; }

void Camera::PrintView() {
    cout << "Current view settings:" << endl;
    cout << "Vector3 m_position = Vector3(" << m_position.x << "f, "
         << m_position.y << "f, " << m_position.z << "f);" << endl;
    cout << "float m_yaw = " << m_yaw << "f, m_pitch = " << m_pitch << "f;"
         << endl;

    cout << "AppBase::m_camera.Reset(Vector3(" << m_position.x << "f, "
         << m_position.y << "f, " << m_position.z << "f), " << m_yaw << "f, "
         << m_pitch << "f);" << endl;
}

Vector3 Camera::NdcToWorld(Vector3 ndc) { 



        Matrix viewRow = GetViewRow();

        if (m_UsingCharacterView && GetTarget() != nullptr)
                viewRow = GetCharacterViewRow();
         
        Matrix projRow = GetProjRow();
        Matrix inverseProjView = (viewRow * projRow).Invert();

        return Vector3::Transform(ndc, inverseProjView);
}

Vector3 Camera::GetNdcDir(Vector2 ndc) { 
        Vector3 a = NdcToWorld(Vector3(ndc.x, ndc.y, 0.0f));
        Vector3 b = NdcToWorld(Vector3(ndc.x, ndc.y, 1.0f));
        Vector3 c = b - a;
        c.Normalize();
        return c;
}

Vector3 Camera::GetForwardVector() { return m_forwardDir; }

Vector3 Camera::GetPosition() { return m_position; }

Matrix Camera::GetProjRow(bool frustumProj) {
          
        if (frustumProj == false) 
                return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
                                    m_aspect, m_nearZ, m_farZ);
        else 
                return XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(m_projFrustumAngleY),
                    m_projFrustumAspect,
                    m_nearZ,
                    m_farZ);

}
Matrix Camera::GetShadowProjRow(Vector2 aspect, float farZ) {
    return XMMatrixOrthographicOffCenterLH(aspect.x, aspect.y, aspect.x,
                                           aspect.y, m_nearZ, farZ);

}


} // namespace hlab