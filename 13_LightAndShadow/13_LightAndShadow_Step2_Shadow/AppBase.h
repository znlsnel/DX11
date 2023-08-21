#pragma once

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "BasicConstantData.h"
#include "BasicMeshGroup.h"
#include "Camera.h"
#include "D3D11Utils.h"
#include "PostProcess.h"
#include "TextureBuffer.h"

namespace hlab {

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

class AppBase {
  public:
    AppBase();
    virtual ~AppBase();

    int Run();
    float GetAspectRatio() const;

    virtual bool Initialize();
    virtual void UpdateGUI() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    virtual void OnMouseMove(int mouseX, int mouseY);
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void UpdateEyeViewProjBuffers(const Vector3 &eyeWorld,
                                  const Matrix &viewRow, const Matrix &projRow,
                                  const Matrix &refl);

    void CreateDepthBuffers();

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();
    void CreateBuffers();
    void SetViewport();
    void CheckMSAA();

  public:
    // 변수 이름 붙이는 규칙은 VS DX11/12 기본 템플릿을 따릅니다.
    // 변수 이름을 줄이기 위해 d3d는 생략했습니다.
    // 예: m_d3dDevice -> m_device
    int m_screenWidth; // 렌더링할 최종 화면의 해상도
    int m_screenHeight;
    int m_guiWidth = 0;
    HWND m_mainWindow;
    bool m_useMSAA = true;
    UINT m_targetSampleCount = 32; // 원하는 MSAA
    UINT m_sampleCount = 1;
    UINT m_numQualityLevels = 1;
    bool m_drawAsWire = false;
    bool m_useEnv = false;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

    // 삼각형 레스터화 -> float(MSAA) -> resolved(No MSAA)
    // -> 후처리(블룸, 톤매핑) -> backBuffer(최종 SwapChain Present)
    // ComPtr<ID3D11Texture2D> m_floatBuffer;
    // ComPtr<ID3D11Texture2D> m_resolvedBuffer;
    // ComPtr<ID3D11RenderTargetView> m_floatRTV;
    // ComPtr<ID3D11RenderTargetView> m_resolvedRTV;
    // ComPtr<ID3D11ShaderResourceView> m_floatSRV;
    // ComPtr<ID3D11ShaderResourceView> m_resolvedSRV;

    TextureBuffer m_mainBuffer; // TextureBuffer 하나에 모아두기

    bool m_useShadow = true;
    bool m_lightRotate = false;
    int m_shadowWidth = 1280 * 2;
    int m_shadowHeight = 1280 * 2;
    TextureBuffer m_shadowBuffer;               // 그림자 맵에 사용
    ComPtr<ID3D11DepthStencilView> m_shadowDSV; // 해상도가 다를 경우

    ComPtr<ID3D11RasterizerState> m_solidRS;
    ComPtr<ID3D11RasterizerState> m_solidCCWRS; // Counter-ClockWise
    ComPtr<ID3D11RasterizerState> m_wireRS;
    ComPtr<ID3D11RasterizerState> m_wireCCWRS;

    // Depth buffer 관련
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    // 거울 관련 (반사는 한 물체, 보통 바닥에만 사용한다고 가정)
    ComPtr<ID3D11DepthStencilState> m_drawDSS; // 일반적으로 그리기
    ComPtr<ID3D11DepthStencilState> m_maskDSS; // 스텐실버퍼에 표시
    ComPtr<ID3D11DepthStencilState> m_drawMaskedDSS; // 스텐실 표시된 곳만
    ComPtr<ID3D11BlendState> m_mirrorBS;
    shared_ptr<BasicMeshGroup> m_mirror;
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // Opacity

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라 클래스 추가
    Camera m_camera;
    bool m_useFirstPersonView = false;

    bool m_keyPressed[256] = {
        false,
    };

    bool m_leftButton = false;
    bool m_rightButton = false;
    bool m_dragStartFlag = false;
    bool m_selected = false;

    // 마우스 커서 위치 저장 (Picking에 사용)
    float m_cursorNdcX = 0.0f;
    float m_cursorNdcY = 0.0f;

    // 후처리 필터
    PostProcess m_postProcess;

    // 거울 구현을 더 효율적으로 하기 위해 ConstBuffer들 분리
    EyeViewProjConstData m_eyeViewProjConstData;
    EyeViewProjConstData m_mirrorEyeViewProjConstData;
    ComPtr<ID3D11Buffer> m_eyeViewProjConstBuffer;
    ComPtr<ID3D11Buffer> m_mirrorEyeViewProjConstBuffer;

    // 카메라 시점 Depth 출력용
    EyeViewProjConstData m_depthEyeViewProjConstData;
    ComPtr<ID3D11Buffer> m_depthEyeViewProjConstBuffer;

    // 조명 시점 Depth 출력용
    EyeViewProjConstData m_lightEyeViewProjConstData;
    ComPtr<ID3D11Buffer> m_lightEyeViewProjConstBuffer;
};
} // namespace hlab