#pragma once

#include "InputManager.h"
#include "AppBase.h"
#include "JsonManager.h"


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

        } 
        else 
        {
                m_appBase->m_leftButton = false;
                if (m_appBase->timeSeconds - lastPressTime < clickAllowTime) {

                        switch (m_appBase->m_mouseMode) { 
                        case EMouseMode::None:
                            break;
                        case EMouseMode::ObjectPickingMode:
                                m_appBase->MouseObjectPicking();
                            break;
                        case EMouseMode::HeightMapEditMode:
                            m_appBase->RayTracing();
                            break;
                        }

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

                if (m_appBase->m_keyPressed[17]) { // ctrl
                        InputCtrl(true, key);
                }

        } else {
                if (key == 'P') { // 애니메이션 일시중지할 때 사용
                        m_appBase->m_pauseAnimation =
                            !m_appBase->m_pauseAnimation;
                }
                if (key == 'Z') { // 카메라 설정 화면에 출력
                        m_appBase->m_camera->PrintView();
                }
                m_appBase->m_keyPressed[key] = false;
        }

}

void hlab::InputManager::InputCtrl(bool isPress, char key) {
        if (m_appBase->m_keyPressed['F']) {
                m_appBase->m_camera->m_objectTargetCameraMode =
                    !m_appBase->m_camera->m_objectTargetCameraMode;
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
                m_appBase->m_mouseMode = EMouseMode::None;
        }
        if (m_appBase->m_keyPressed['2']) {
                m_appBase->m_mouseMode = EMouseMode::ObjectPickingMode;
        }
        if (m_appBase->m_keyPressed['3']) {
                m_appBase->m_mouseMode = EMouseMode::HeightMapEditMode;
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
