#pragma once

namespace hlab{

	enum EEditTransformMode { 
                none,
                position,
                rotation,
                scale,
        };

class InputManager {
  public:
    InputManager(class AppBase *appBase){ m_appBase = appBase;
    };
    void Update(float dt);
    void InputLeftMouse(bool isPress, int mouseX = -1 , int mouseY = -1);
    void InputRightMouse(bool isPress, int mouseX = -1, int mouseY = -1);
    void InputKey(bool isPress, char key);
    void InputCtrl(bool isPress, char key);
    void InputMouseWheel(float wheelDt);

    void UpdateObjectTransform(EEditTransformMode mode, bool x, bool y, bool z, float dt);
    private:
    class AppBase *m_appBase;
      bool usingRayCasting = false;
     
      EEditTransformMode editTransformMode = EEditTransformMode::none;
};
}
