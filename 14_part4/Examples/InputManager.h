#pragma once

namespace hlab{

class InputManager {
  public:
    InputManager(class AppBase *appBase){ m_appBase = appBase;
    };
     
    void InputLeftMouse(bool isPress, int mouseX = -1 , int mouseY = -1);
    void InputRightMouse(bool isPress, int mouseX = -1, int mouseY = -1);
    void InputKey(bool isPress, char key);
    void InputCtrl(bool isPress, char key);
    void InputMouseWheel(float wheelDt);

    private:
    class AppBase *m_appBase;

    
};
}
