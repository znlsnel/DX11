#pragma once

#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>
#include <map>


#include "Camera.h"
#include "ComputePSO.h"
#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GraphicsPSO.h"
#include "Model.h"
#include "PostProcess.h"
#include "Timer.h"

namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Ray;
using DirectX::SimpleMath::Vector3;
using DirectX::XMFLOAT3;
using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;
using std::map;
using std::pair;

struct BVNode {

  public:
    BVNode(){};
    BVNode(BoundingBox& bx) { 
            boundingBox = bx;
    };
    int objectID = -1;
    int leftChildID = -1;
    int rightChildID = -1; 
    BoundingBox boundingBox;
     
};


enum EMouseMode : int {
        MouseModeNone = 0,
        ObjectPickingMode = 1,
        TextureMapEditMode = 2,
};

enum EEditTextureType : int {
        EditTextureTypeNone = -1,
        Ground = 0,
        RiverBed = 1,
        PavingStones = 2,
        Rock = 3,
};

class AppBase {

    struct MyFrustum {
        public:
                MyFrustum(){};
          float Distance(Vector3 planePoint, Vector3 normal,
                               Vector3 point) {
                    return normal.Dot(point - planePoint);
          };
                void InitFrustum(AppBase* appBase) {
                    frustum.clear();  
                    frustum.resize(0);  
                    shared_ptr<Camera> camera = appBase->m_camera;
                    Vector3 frontLeftTop =
                        camera->NdcToWorld(Vector3(-1.0f, 1.0f, 0.0f));
                    Vector3 frontRightTop =
                        camera->NdcToWorld(Vector3(1.0f, 1.0f, 0.0f));
                    Vector3 frontLeftBottom =
                        camera->NdcToWorld(Vector3(-1.0f, -1.0f, 0.0f));
                    Vector3 frontRightBottom =
                        camera->NdcToWorld(Vector3(1.0f, -1.0f, 0.0f));

                    Vector3 backLeftTop =
                        camera->NdcToWorld(Vector3(-1.0f, 1.0f, 1.0f));
                    Vector3 backRightTop =
                        camera->NdcToWorld(Vector3(1.0f, 1.0f, 1.0f));
                    Vector3 backLeftBottom =
                        camera->NdcToWorld(Vector3(-1.0f, -1.0f, 1.0f));
                    Vector3 backRightBottom =
                        camera->NdcToWorld(Vector3(1.0f, -1.0f, 1.0f));
                      
                    Vector3 origin, normal;
                    auto CraeteOriginNNormal = [&](Vector3 &leftBottom,
                                                   Vector3 &leftTop,
                                                   Vector3 &rightBottom, Vector3 &rightTop) {
                        normal = (leftTop - leftBottom)
                                     .Cross(rightBottom - leftBottom);
                        normal.Normalize();

                /*        origin = leftBottom + (leftTop - leftBottom) * 0.5f +
                                 (rightBottom - leftBottom) * 0.5f;*/
                        origin =
                            (leftBottom + leftTop + rightBottom + rightTop) *
                            0.25f;
                    };
                     
                    // front
                    CraeteOriginNNormal(frontLeftBottom, frontLeftTop,
                                        frontRightBottom, frontRightTop);
                    frustum.push_back(
                        std::make_pair(SimpleMath::Plane(origin, normal), origin));
                     
                    // Back
                    CraeteOriginNNormal(backRightBottom, backRightTop,
                                        backLeftBottom, backRightTop);
                    frustum.push_back(std::make_pair(
                        SimpleMath::Plane(origin, normal), origin));
                    // Left
                    CraeteOriginNNormal(backLeftBottom, backLeftTop,
                                        frontLeftBottom, frontLeftTop);
                    frustum.push_back(std::make_pair(
                        SimpleMath::Plane(origin, normal), origin));
                    // Right 
                    CraeteOriginNNormal(frontRightBottom, frontRightTop,
                                        backRightBottom, backRightTop); 
                    frustum.push_back(std::make_pair(
                        SimpleMath::Plane(origin, normal), origin));
                    // Bottom
                    CraeteOriginNNormal(frontRightBottom, backRightBottom,
                                        frontLeftBottom, backLeftBottom);
                    frustum.push_back(std::make_pair(
                        SimpleMath::Plane(origin, normal), origin));
                    // Top
                    CraeteOriginNNormal(frontLeftTop, backLeftTop,
                                        frontRightTop, backRightTop);
                    frustum.push_back(std::make_pair( 
                        SimpleMath::Plane(origin, normal), origin));
                }
                bool Intersects(Vector3 Center, Vector3 Extents, Matrix& row) {
                    Vector3 corners[8] = {
                        Center + Vector3(-1.0f, -1.0f, -1.0f) * Extents,
                        Center + Vector3(-1.0f, 1.0f, -1.0f) * Extents,
                        Center + Vector3(1.0f, 1.0f, -1.0f) * Extents,
                        Center + Vector3(1.0f, -1.0f, -1.0f) * Extents,
                        Center + Vector3(-1.0f, -1.0f, 1.0f) * Extents,
                        Center + Vector3(-1.0f, 1.0f, 1.0f) * Extents,
                        Center + Vector3(1.0f, 1.0f, 1.0f) * Extents,
                        Center + Vector3(1.0f, -1.0f, 1.0f) * Extents
                      
                    }; 
                        for (int i = 0; i < 8; i++)
                        corners[i] = Vector3::Transform(corners[i], row);
                   
                    
                     
                    for (int i = 0; i < frustum.size(); i++) {
                        Vector3 planePoint = frustum[i].second;
                        Vector3 planeNormal = frustum[i].first.Normal();
                        int inCount = 0;
                        int outCount = 0;
                         
                        for (int j = 0; j < 8; j++) {
                            if (Distance(planePoint, planeNormal, corners[j]) >
                                0) {
                                outCount++; 
                            } else
                                inCount++;
                        }
                         
                        if (inCount == 0)
                            return false;          
                    }    
                     
                    return true; 
                };
                vector<pair<SimpleMath::Plane, Vector3>> frustum;

        }; 

  public:
    AppBase();
    virtual ~AppBase(); 

    int Run();
    float GetAspectRatio() const;

    virtual bool Initialize();
    virtual bool InitScene();
    virtual void UpdateGUI(); 
    virtual void Update(float dt);  
    virtual void UpdateLights(float dt);
    void UpdateLightInfo(ComPtr<ID3D11Buffer>& shadowGlobalConstsGPU, GlobalConstants& shadowGlobalConstants, Light &light, Vector3& aspect);
    void UpdateBVH(); 
     
     
    virtual void RenderDepthOnly();
    virtual void RenderShadowMaps();
    virtual void RenderOpaqueObjects();
    virtual void RenderMirror();
    virtual void Render();
    void PostRender();
    void RenderBVH();
      
    void SetPipelineState(const GraphicsPSO &pso);
    void SetPipelineState(const ComputePSO &pso);
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);
    virtual void SetHeightPosition(Vector3 origin, Vector3 dir, float &dist);
    void SetMainViewport();
    void SetShadowViewport();

    void GetObjectsInFrustum();

    virtual void OnMouseMove(int mouseX, int mouseY);
    virtual void OnMouseClick(int mouseX, int mouseY);

    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void ResizeSwapChain(int width, int height);

    void InitCubemaps(wstring basePath, wstring envFilename,
                      wstring specularFilename, wstring irradianceFilename,
                      wstring brdfFilename);
    void UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                               const Matrix &viewRow, const Matrix &projRow,
                               const Matrix &refl = Matrix());
     
    void CreateDepthBuffers();

    shared_ptr<Model> PickClosest(const Ray &pickingRay, float &minDist);
    void ProcessMouseControl();
    void DestroyObject(shared_ptr<class Model> object);
    bool MouseObjectPicking();
     
    virtual Vector3 RayCasting(bool editTexture = false, float mouseNdcX = -10.f, float mouseNdcY = -10.0f);
    virtual void RayCasting(Vector3 origin, Vector3 dir, float& dist);
     
    void replicateObject();
    void AddBasicList(shared_ptr<Model>& object, bool basicList = true, bool editable = true, bool saveable = false);
    bool IsMouseHoveringImGui(); 
    void ComputeShaderBarrier();

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();
    void CreateBuffers();


    virtual void ObjectDrag();
    template <typename T>
    bool ReadPixelOfMousePos(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context);

  public:
    // 변수 이름 붙이는 규칙은 VS DX11/12 기본 템플릿을 따릅니다.
    // 변수 이름을 줄이기 위해 d3d는 생략했습니다.
    // 예: m_d3dDevice -> m_device
           
        Vector2 ImPos;
        Vector2 ImSize;
        
    int m_imGuiWidth = 0;
    int m_screenWidth = 1920;
    int m_screenHeight = 720;
    HWND m_mainWindow;
    bool m_useMSAA = true;
    UINT m_numQualityLevels = 0;
    bool m_drawAsWire = false;
    bool m_drawOBB = false; // Draw Object Oriented Bounding Box
    bool m_drawBS = false;  // Draw Bounding Sphere
    bool m_selected = false;
    bool m_bRenderingBVH = false;
    float frustumSize = 1.0f;

    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

    // 삼각형 레스터화 -> float(MSAA) -> resolved(No MSAA)
    // -> 후처리(블룸, 톤매핑) -> backBuffer(최종 SwapChain Present)

    ComPtr<ID3D11Texture2D> m_floatBuffer;
    ComPtr<ID3D11Texture2D> m_resolvedBuffer;
    ComPtr<ID3D11Texture2D> m_postEffectsBuffer;
    ComPtr<ID3D11Texture2D> m_prevBuffer; // 간단한 모션 블러 효과
    ComPtr<ID3D11Texture2D> m_texArray;
    ComPtr<ID3D11Texture2D> m_tempTexture; 

    ComPtr<ID3D11Texture2D> m_indexTexture;
    ComPtr<ID3D11Texture2D> m_indexTempTexture;
    ComPtr<ID3D11Texture2D> m_indexStagingTexture;
    ComPtr<ID3D11RenderTargetView> m_indexRenderTargetView;


    ComPtr<ID3D11RenderTargetView> m_floatRTV;
    ComPtr<ID3D11RenderTargetView> m_resolvedRTV;
    ComPtr<ID3D11RenderTargetView> m_postEffectsRTV;
    ComPtr<ID3D11RenderTargetView> m_prevRTV;
    ComPtr<ID3D11ShaderResourceView> m_resolvedSRV;
    ComPtr<ID3D11ShaderResourceView> m_postEffectsSRV;
    ComPtr<ID3D11ShaderResourceView> m_prevSRV;
    ComPtr<ID3D11ShaderResourceView> m_billboardTreeSRV;

    Matrix tempRotation;

    // Depth buffer 관련
    ComPtr<ID3D11Texture2D> m_depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_depthOnlyDSV;
    ComPtr<ID3D11DepthStencilView> m_defaultDSV;
    ComPtr<ID3D11ShaderResourceView> m_depthOnlySRV;

    // Shadow maps
    int m_shadowWidth = 1280;
    int m_shadowHeight = 1280;

    ComPtr<ID3D11Texture2D> m_shadowBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11Texture2D> m_overallShadowBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowDSVs[MAX_LIGHTS];
    ComPtr<ID3D11DepthStencilView> m_overallShadowDSV;
    ComPtr<ID3D11ShaderResourceView> m_shadowSRVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> m_overallShadowSRV;

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라 클래스 추가
    shared_ptr<Camera> m_camera;
    bool m_keyPressed[256] = {
        false,
    };

        float m_keyPressedTime[256] = {
        0.0f,
    };

    bool m_keyToggle[256] = {
        false,
    };

    bool m_leftButton = false;
    bool m_rightButton = false;
    bool m_dragStartFlag = false;
    int maxBVHRenderLevel = 0;

    // 마우스 커서 위치 저장 (Picking에 사용)
    float m_mouseNdcX = 0.0f;
    float m_mouseNdcY = 0.0f;
    int m_mouseX = -1;
    int m_mouseY = -1;
    int m_preMouse[2] = {0, 0};
    float cameraDistance_min = 0.3f;
    float cameraDistance_max = 10.f;

    float cameraSpeed_min = 0.01f;
    float cameraSpeed_max = 2.0f;
    Vector3 testPos;

    double timeSeconds = 0.0;
    float m_dt = 0.0f;
    // 렌더링 -> PostEffects -> PostProcess
    PostEffectsConstants m_postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> m_postEffectsConstsGPU;
     
    PostProcess m_postProcess;

    // 다양한 Pass들을 더 간단히 구현하기 위해 ConstBuffer들 분리
    GlobalConstants m_globalConstsCPU;
    GlobalConstants m_reflectGlobalConstsCPU;
    GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_globalConstsGPU;
    ComPtr<ID3D11Buffer> m_reflectGlobalConstsGPU; 
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];
      
    // 공통으로 사용하는 텍스춰들 
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;  
    ComPtr<ID3D11ShaderResourceView> m_specularSRV; 
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV; 
            
    bool m_lightRotate = false;             
    bool m_pauseAnimation = false;     
     
    vector<uint8_t> heightMapImage; 
          
    // shadow 0.3 -> 1.0 -> 3.0 -> 5.0 -> (10.0 * 10.0) 
    Vector3 m_shadowAspects[5] =   
    {
            Vector3(0.8f, -0.8f, 6.f),   
            Vector3(2.0f, -2.0f, 6.5f),  
            Vector3(3.5f, -3.5f, 8.f),   
            Vector3(7.0f, -7.0f, 10.f),  
            Vector3(10.0f, -10.0f, 200.f),  
    };   
         
    // 여러 예제들 공용     
    shared_ptr<Model> m_screenSquare; // PostEffect에 사용   
    shared_ptr<Model> m_skybox;
    shared_ptr<Model> m_pickedModel; 
    shared_ptr<Model> m_lightSphere[MAX_LIGHTS];
    shared_ptr<Model> m_mirror; // 거울은 별도로 그림
    shared_ptr<class TessellationModel> m_groundPlane; 
    shared_ptr<class InputManager> m_inputManager;

    vector<shared_ptr<Model>> m_cursorSphere;
    vector<BVNode> m_BVNodes; 
    vector<shared_ptr<Mesh>> m_BVNMeshs;
    int m_BVHNodeID = 0;


    DirectX::SimpleMath::Plane m_mirrorPlane; 
    float m_mirrorAlpha = 1.0f; // Opacity

    EMouseMode m_mouseMode = EMouseMode::ObjectPickingMode;
    EEditTextureType m_textureType = EEditTextureType::EditTextureTypeNone;

    // 거울이 아닌 물체들의 리스트 (for문으로 그리기 위함)
    map<int, shared_ptr<Model>> m_objects;
    vector<shared_ptr<Model>> m_basicList;
    vector<shared_ptr<Model>> m_NoneBVHList; 
    vector<shared_ptr<Model>> m_foundModelList;
    vector<shared_ptr<class Character>> m_characters;
     
    shared_ptr<class JsonManager> m_JsonManager;
    map < string, tuple<ComPtr<ID3D11Texture2D>,
            ComPtr<ID3D11ShaderResourceView>>> m_textureStorage; 
}; 

} // namespace hlab