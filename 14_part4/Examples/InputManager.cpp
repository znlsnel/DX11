#pragma once

#include "InputManager.h"
#include "AppBase.h"
#include "JsonManager.h"

#include <directxtk/SimpleMath.h>


void hlab::InputManager::Update(float dt) 
{ 
    static float rayTime = 0.0f;
    if (usingRayCasting && rayTime > 1.0f / 60.f){
        if (m_appBase->m_mouseMode == EMouseMode::TextureMapEditMode)
                m_appBase->RayCasting(true); 

        rayTime = 0.0f;        


    } 
    else if (usingRayCasting == false){
                m_appBase->m_cursorSphere[0]->m_isVisible = false;
    }  
        rayTime += dt; 

        if (m_appBase->m_leftButton)
                UpdateObjectTransform(
                        editTransformMode, 
                        m_appBase->m_keyPressed['Z'],
                        m_appBase->m_keyPressed['X'],
                        m_appBase->m_keyPressed['C'],
                        dt
                );

}

void hlab::InputManager::InputLeftMouse(bool isPress, int mouseX, int mouseY) {

        static float lastPressTime = 0.0;
        const float clickAllowTime = 0.2f;

        if (m_appBase->IsMouseHoveringImGui())
            return;

        if (isPress) 
        {
            m_appBase->OnMouseClick(mouseX, mouseY);

                lastPressTime = m_appBase->timeSeconds;
                if (!m_appBase->m_leftButton) 
                {
                        m_appBase->m_dragStartFlag =   true; 
                }
                m_appBase->m_leftButton = true;

                switch (m_appBase->m_mouseMode) {
                case EMouseMode::MouseModeNone:
                        break;
                case EMouseMode::ObjectPickingMode:
                        break;
                case EMouseMode::TextureMapEditMode:
                        usingRayCasting = true;
                        break;
                }

        } 
        else 
        {
                m_appBase->m_leftButton = false;
                if (m_appBase->timeSeconds - lastPressTime < clickAllowTime) {

                        switch (m_appBase->m_mouseMode) { 
                        case EMouseMode::MouseModeNone:
                            break;
                        case EMouseMode::ObjectPickingMode:
                                m_appBase->MouseObjectPicking();
                            break;
                        case EMouseMode::TextureMapEditMode:

                            break;
                        }

                }

                
                switch (m_appBase->m_mouseMode) {
                case EMouseMode::MouseModeNone:
                        break;
                case EMouseMode::ObjectPickingMode:
                        break;
                case EMouseMode::TextureMapEditMode:
                        usingRayCasting = false;
                        break;
                }
        }
}

void hlab::InputManager::InputRightMouse(bool isPress, int mouseX, int mouseY) {
        if (isPress) {
                m_appBase->m_preMouse[0] = mouseX;
                m_appBase->m_preMouse[1] = mouseY;
                m_appBase->m_camera->m_isCameraLock = false;

                if (!m_appBase->m_rightButton) {
                        m_appBase->m_dragStartFlag =
                            true; // 드래그를 새로 시작하는지 확인
                }
                m_appBase->m_rightButton = true;
        } else {

                m_appBase->m_camera->m_isCameraLock = true;
                m_appBase->m_rightButton = false;
        
        }

}

void hlab::InputManager::InputKey(bool isPress, char key) {

        
        if (isPress) {
                m_appBase->m_keyPressed[key] = true;
                m_appBase->m_keyToggle[key] = !m_appBase->m_keyToggle[key];
                //cout << key << "\n";
                
                if (m_appBase->m_keyPressed[17]) { // ctrl
                        InputCtrl(true, key);
                }
                if (m_appBase->m_keyPressed[VK_DELETE]) {
                        if (m_appBase->m_keyPressedTime[VK_DELETE] ==  0.0f)
                                m_appBase->m_keyPressedTime[VK_DELETE] =
                                        m_appBase->timeSeconds;
                }
        } else {
                if (key == 'P') { // 애니메이션 일시중지할 때 사용
                        m_appBase->m_pauseAnimation =
                            !m_appBase->m_pauseAnimation;
                }
                if (key == 'Z') { // 카메라 설정 화면에 출력
                        m_appBase->m_camera->PrintView();
                }
                if (m_appBase->m_keyPressed[VK_DELETE]) {
                        if (m_appBase->m_pickedModel &&
                            m_appBase->timeSeconds - m_appBase
                                    ->m_keyPressedTime[VK_DELETE] < 0.5f)
                        {
                            m_appBase->DestroyObject(m_appBase->m_pickedModel);
                        } 
                        m_appBase->m_keyPressedTime[VK_DELETE] = 0.0f;
                }

                m_appBase->m_keyPressed[key] = false;
        }

}

void hlab::InputManager::InputCtrl(bool isPress, char key) {
        if (m_appBase->m_keyPressed['F']) {
                m_appBase->m_camera->m_objectTargetCameraMode =
                    !m_appBase->m_camera->m_objectTargetCameraMode;
                if (m_appBase->m_camera->GetTarget() == nullptr)
                        m_appBase->m_camera->m_objectTargetCameraMode = false;
        }
         
        if (m_appBase->m_keyPressed['S']) {
                m_appBase->m_JsonManager->SaveMesh();
                m_appBase->m_keyPressed['S'] = false;
        }

        if (m_appBase->m_keyPressed['D']) {
                m_appBase->replicateObject();
                m_appBase->m_keyPressed['D'] = false;
        }

        if (m_appBase->m_keyPressed['1']) {
                m_appBase->m_mouseMode = EMouseMode::MouseModeNone;
        }
        if (m_appBase->m_keyPressed['2']) {
                m_appBase->m_mouseMode = EMouseMode::ObjectPickingMode;
        }
        if (m_appBase->m_keyPressed['3']) {
                m_appBase->m_mouseMode = EMouseMode::TextureMapEditMode;
        }

        if (m_appBase->m_pickedModel != nullptr) {


                if (m_appBase->m_camera->m_objectTargetCameraMode == false)
                {
                        bool pressQ = m_appBase->m_keyPressed['Q'];
                        bool pressW = m_appBase->m_keyPressed['W'];
                        bool pressE= m_appBase->m_keyPressed['E'];

                        if (m_appBase->m_keyPressed['Q']) 
                            editTransformMode =
                                editTransformMode ==
                                        EEditTransformMode::position
                                       ? EEditTransformMode::none
                                       : EEditTransformMode::position;
                        else if (m_appBase->m_keyPressed['W'])
                            editTransformMode =
                                editTransformMode ==
                                        EEditTransformMode::rotation
                                       ? EEditTransformMode::none
                                       : EEditTransformMode::rotation;
                        else if (m_appBase->m_keyPressed['E'])
                            editTransformMode =
                                editTransformMode == EEditTransformMode::scale
                                       ? EEditTransformMode::none
                                       : EEditTransformMode::scale;

                        if (pressQ || pressW || pressE)
                        {
                            string t = editTransformMode ==
                                               EEditTransformMode::position
                                           ? "position"
                                       : editTransformMode ==
                                               EEditTransformMode::rotation
                                           ? "rotation"
                                : editTransformMode == EEditTransformMode::scale
                                           ? "scale"
                                           : "none";
                             
                            cout << "Curr EditTransformMode : " << t << "\n";
                        }
                }
        } 
}

void hlab::InputManager::InputMouseWheel(float wheelDt) {

        if (m_appBase->IsMouseHoveringImGui())
                return;

        int sign = wheelDt > 0.0f ? 1 : -1;
        if (m_appBase->m_camera->m_objectTargetCameraMode) {
                m_appBase->m_camera->cameraDistance +=
                    (m_appBase->cameraDistance_max -
                     m_appBase->cameraDistance_min) *
                    0.05f * sign;
                m_appBase->m_camera->cameraDistance = std::clamp(m_appBase->m_camera->cameraDistance,
                               m_appBase->cameraDistance_min,
                    m_appBase->cameraDistance_max);
        } else {
                m_appBase->m_camera->cameraSpeed +=
                    (m_appBase->cameraSpeed_max - m_appBase->cameraSpeed_min) *  0.05f * sign;

                m_appBase->m_camera->cameraSpeed = std::clamp(m_appBase->m_camera->cameraSpeed,
                    m_appBase->cameraSpeed_min, m_appBase->cameraSpeed_max);
        }
}

void hlab::InputManager::UpdateObjectTransform(EEditTransformMode mode, bool x, bool y, bool z, float dt) {

        static Vector2 prevMouseNdcPos;
        static Vector3 prevNdcWorldPosX; 
        static Vector3 prevNdcWorldPosY; 
        static Vector3 prevNdcWorldPosZ; 

        if ((!x && !y && !z) || mode == EEditTransformMode::none || m_appBase->m_pickedModel == nullptr){
                prevMouseNdcPos = Vector2(0.0f, 0.0f);
                prevNdcWorldPosX = Vector3(0.0f, 0.0f, 0.0f);
                prevNdcWorldPosY = Vector3(0.0f, 0.0f, 0.0f);
                prevNdcWorldPosZ = Vector3(0.0f, 0.0f, 0.0f); 
                return;
        }
         
        if (prevMouseNdcPos == Vector2(0.0f, 0.0f)) {
                prevMouseNdcPos =
                    Vector2(m_appBase->m_mouseNdcX, m_appBase->m_mouseNdcY);
                return;        
        }
          
        Vector2 currMouseNdcPos =
            Vector2(m_appBase->m_mouseNdcX, m_appBase->m_mouseNdcY);

        Vector3 objectPos = m_appBase->m_pickedModel->GetPosition();
        Vector3 objectRotation =  m_appBase->m_pickedModel->GetRotation();
        Vector3 objectScale = m_appBase->m_pickedModel->GetScale();

        Vector3 rightV = Vector3::Transform(
            Vector3(1.0f, 0.0f, 0.0f),
            Matrix::CreateRotationY(m_appBase->m_camera->GetYaw()));
        rightV = m_appBase->m_camera->GetRightVector();
        Vector3 forwardV = Vector3::Transform(
            Vector3(0.0f, 0.0f, -1.0f),
            Matrix::CreateRotationY(m_appBase->m_camera->GetYaw()));
        Vector3 UpV = Vector3(0.0f, 1.0f, 0.0f);
         
        if (mode == EEditTransformMode::position) 
        {
                Vector3 addPos;
                Vector3 point;  

                if (x) 
                {  
                        Vector2 tempOrigin =
                            Vector2(currMouseNdcPos.x, currMouseNdcPos.y);

                        Vector3 rayOrigin = m_appBase->m_camera->NdcToWorld(
                            Vector3(tempOrigin.x, tempOrigin.y, 0.0f)); 
                        Vector3 rayDir =
                            m_appBase->m_camera->GetNdcDir(tempOrigin);
                        
                        SimpleMath::Ray ray = SimpleMath::Ray(rayOrigin, rayDir);
                        SimpleMath::Plane p(objectPos, UpV);
                        float dist = 0.0f; 
                        bool result = ray.Intersects(p, dist);
                        prevNdcWorldPosY = Vector3();
                        if (result) 
                        {
                            point = rayOrigin + rayDir * dist;
                            if (prevNdcWorldPosX.Length() > 0.0f) 
                            {
                                Vector3 dir = point - prevNdcWorldPosX;
                                addPos += dir; 
                            }
                            prevNdcWorldPosX = point;
                        } 
                } 
                else if (y) 
                {
                        Vector2 tempOrigin =
                            Vector2(currMouseNdcPos.x, currMouseNdcPos.y);

                        Vector3 rayOrigin = m_appBase->m_camera->NdcToWorld(
                            Vector3(tempOrigin.x, tempOrigin.y, 0.0f));
                        Vector3 rayDir =
                            m_appBase->m_camera->GetNdcDir(tempOrigin);

                        SimpleMath::Ray ray =
                            SimpleMath::Ray(rayOrigin, rayDir);
                        SimpleMath::Plane p(objectPos, -forwardV);
                        float dist = 0.0f;
                        bool result = ray.Intersects(p, dist);
                         
                        prevNdcWorldPosX = Vector3();
                        if (result) {
                                point = rayOrigin + rayDir * dist;
                                if (prevNdcWorldPosY.Length() > 0.0f) {
                                Vector3 dir = point - prevNdcWorldPosY;
                                addPos += dir;
                                }
                                prevNdcWorldPosY = point;
                        } 
                } else {
                        prevNdcWorldPosX = Vector3(0.0f, 0.0f, 0.0f);
                        prevNdcWorldPosY = Vector3(0.0f, 0.0f, 0.0f);
                }

                if (addPos.Length() > 0.001f)
                        m_appBase->m_pickedModel->UpdatePosition(
                            objectPos + addPos); 

                 
        } 
        else if (mode == EEditTransformMode::rotation) {
                float speed = 3.0f;
                if (m_appBase->m_keyPressed[VK_SHIFT])
                        speed = 0.1f;
                
                if (x) {
                        float dir = -1.0f;
                        if (prevMouseNdcPos.y > currMouseNdcPos.y) {
                                dir = 1.0f;
                        } else if (prevMouseNdcPos.y == currMouseNdcPos.y) {
                                dir = 0.0f;
                        }

                        m_appBase->m_pickedModel->UpdateRotation(
                            objectRotation +
                            Vector3(1.0f, 0.0f, 0.0f) * speed * dir * dt);
                } 
                if (y) {
                        float dir = -1.0f;
                        if (prevMouseNdcPos.x > currMouseNdcPos.x) {
                                dir = 1.0f;
                        } else if (prevMouseNdcPos.x == currMouseNdcPos.x) {
                                dir = 0.0f;
                        }
                        m_appBase->m_pickedModel->UpdateRotation(
                            objectRotation +
                            Vector3(0.0f, 1.0f, 0.0f) * speed * dir * dt); 
                } else if (z) { 
                        float dir = 1.0f;
                        if (prevMouseNdcPos.x > currMouseNdcPos.x) {
                                dir = -1.0f;
                        } else if (prevMouseNdcPos.x == currMouseNdcPos.x) {
                                dir = 0.0f;
                        }
                        m_appBase->m_pickedModel->UpdateRotation(
                            objectRotation +
                            Vector3(0.0f, 0.0f, 1.0f) * speed * dir * dt);
                } 
        }
        else if (mode == EEditTransformMode::scale) 
        { 
                float speed = 3.0f;
                if (m_appBase->m_keyPressed[VK_SHIFT])
                        speed = 0.1f; 

                if (x)  
                {
                        float dir = 1.0f;
                        if (prevMouseNdcPos.x > currMouseNdcPos.x) {
                            dir = -1.0f;
                        } else if (prevMouseNdcPos.x == currMouseNdcPos.x) {
                            dir = 0.0f;
                        }

                        m_appBase->m_pickedModel->UpdateScale(
                            objectScale +
                            Vector3(1.0f, 0.0f, 0.0f) * speed * dir * dt);
                }
                if (y) {
                        float dir = 1.0f;
                        if (prevMouseNdcPos.y > currMouseNdcPos.y) {
                            dir = -1.0f;
                        } else if (prevMouseNdcPos.y == currMouseNdcPos.y) {
                            dir = 0.0f;
                        }
                        m_appBase->m_pickedModel->UpdateScale(
                            objectScale +
                            Vector3(0.0f, 1.0f, 0.0f) * speed * dir * dt);
                } else if (z) {
                        float dir = 1.0f;
                        if (prevMouseNdcPos.y > currMouseNdcPos.y) {
                            dir = -1.0f;
                        } else if (prevMouseNdcPos.y == currMouseNdcPos.y) {
                            dir = 0.0f;
                        } 
                        m_appBase->m_pickedModel->UpdateScale(
                            objectScale +
                            Vector3(0.0f, 0.0f, 1.0f) * speed * dir * dt);
                } 
        }

        prevMouseNdcPos = currMouseNdcPos;
         
}
