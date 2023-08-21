#include "AppBase.h"

#include <algorithm>

#include "D3D11Utils.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace hlab {

using namespace std;

AppBase *g_appBase = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_screenWidth(1280), m_screenHeight(720), m_mainWindow(0),
      m_screenViewport(D3D11_VIEWPORT()) {

    g_appBase = this;

    m_camera.SetAspectRatio(this->GetAspectRatio());
}

AppBase::~AppBase() {
    g_appBase = nullptr;

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_mainWindow);
    // UnregisterClass(wc.lpszClassName, wc.hInstance);//생략

    // COMPtr에서 알아서 release
    // ComPtr automatically maintains a reference count for the underlying
    // interface pointer and releases the interface when the reference count
    // goes to zero.
    // https:learn.microsoft.com/en-us/cpp/cppcx/wrl/comptr-class?view=msvc-170
    // 예시: m_d3dDevice.Reset(); 생략
}

float AppBase::GetAspectRatio() const {
    return float(m_screenWidth - m_guiWidth) / m_screenHeight;
}

int AppBase::Run() {

    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            // ImGui가 측정해주는 Framerate 출력
            ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            UpdateGUI(); // 추가적으로 사용할 GUI

            m_guiWidth = 0;

            ImGui::End();
            ImGui::Render();

            Update(ImGui::GetIO().DeltaTime);

            Render(); // <- 중요: 우리가 구현한 렌더링

            // GUI 렌더링
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // GUI 렌더링 후에 Present() 호출
            m_swapChain->Present(1, 0);
        }
    }

    return 0;
}

bool AppBase::Initialize() {

    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    if (!InitGUI())
        return false;

    // 콘솔창이 렌더링 창을 덮는 것을 방지
    SetForegroundWindow(m_mainWindow);

    return true;
}

void AppBase::OnMouseMove(int mouseX, int mouseY) {

    // 마우스 커서의 위치를 NDC로 변환
    // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
    // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
    m_cursorNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_cursorNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;

    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
    m_cursorNdcX = std::clamp(m_cursorNdcX, -1.0f, 1.0f);
    m_cursorNdcY = std::clamp(m_cursorNdcY, -1.0f, 1.0f);

    // 카메라 시점 회전
    if (m_useFirstPersonView) {
        m_camera.UpdateMouse(m_cursorNdcX, m_cursorNdcY);
    }
}

LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        // 화면 해상도가 바뀌면 SwapChain을 다시 생성
        if (m_swapChain) {

            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));
            m_guiWidth = 0;

            m_backBufferRTV.Reset();
            m_swapChain->ResizeBuffers(0, // 현재 개수 유지
                                       (UINT)LOWORD(lParam), // 해상도 변경
                                       (UINT)HIWORD(lParam),
                                       DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                                       0);
            CreateBuffers();

            SetViewport();
            m_camera.SetAspectRatio(this->GetAspectRatio());
        }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        if (!m_leftButton) {
            m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        m_leftButton = true;
        break;
    case WM_LBUTTONUP:
        m_leftButton = false;
        break;
    case WM_RBUTTONDOWN:
        if (!m_rightButton) {
            m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        m_rightButton = true;
        break;
    case WM_RBUTTONUP:
        m_rightButton = false;
        break;
    case WM_KEYDOWN:
        m_keyPressed[wParam] = true;
        if (wParam == 27) { // ESC키 종료
            DestroyWindow(hwnd);
        }
        if (wParam == VK_SPACE) {
            m_lightRotate = !m_lightRotate;
        }
        break;
    case WM_KEYUP:
        if (wParam == 70) { // f키 일인칭 시점
            m_useFirstPersonView = !m_useFirstPersonView;
        }

        if (wParam == 67) { // c키 화면 캡쳐
            ComPtr<ID3D11Texture2D> backBuffer;
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
            D3D11Utils::WriteToFile(m_device, m_context, backBuffer,
                                    "captured.png");
        }

        m_keyPressed[wParam] = false;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

// 여러 물체들이 공통적으료 사용하는 Const 업데이트
void AppBase::UpdateEyeViewProjBuffers(const Vector3 &eyeWorld,
                                       const Matrix &viewRow,
                                       const Matrix &projRow,
                                       const Matrix &refl = Matrix()) {

    m_eyeViewProjConstData.eyeWorld = eyeWorld;
    m_eyeViewProjConstData.viewProj = (viewRow * projRow).Transpose();
    m_mirrorEyeViewProjConstData.eyeWorld = eyeWorld;
    m_mirrorEyeViewProjConstData.viewProj =
        (refl * viewRow * projRow).Transpose();

    D3D11Utils::UpdateBuffer(m_device, m_context, m_eyeViewProjConstData,
                             m_eyeViewProjConstBuffer);
    D3D11Utils::UpdateBuffer(m_device, m_context, m_mirrorEyeViewProjConstData,
                             m_mirrorEyeViewProjConstBuffer);

    m_depthEyeViewProjConstData = m_eyeViewProjConstData;
    m_depthEyeViewProjConstData.depthPass = 1; // Depth만 출력할때 사용
    D3D11Utils::UpdateBuffer(m_device, m_context, m_depthEyeViewProjConstData,
                             m_depthEyeViewProjConstBuffer);
}

void AppBase::CreateDepthBuffers() {

    // DepthStencilView 만들기
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = m_screenWidth;
        desc.Height = m_screenHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
        desc.SampleDesc.Count = m_sampleCount;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        ThrowIfFailed(m_device->CreateTexture2D(
            &desc, 0, m_depthStencilBuffer.GetAddressOf()));
        ThrowIfFailed(m_device->CreateDepthStencilView(
            m_depthStencilBuffer.Get(), NULL,
            m_depthStencilView.GetAddressOf()));

        // 그림자맵 해상도를 더 높일 경우
        ComPtr<ID3D11Texture2D> temp;
        desc.Width = m_shadowWidth;
        desc.Height = m_shadowHeight;
        ThrowIfFailed(m_device->CreateTexture2D(&desc, 0, temp.GetAddressOf()));
        ThrowIfFailed(m_device->CreateDepthStencilView(
            temp.Get(), NULL, m_shadowDSV.GetAddressOf()));
    }

    /* D3D11_DEPTH_STENCIL_DESC 옵션 정리
     * https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencil_desc
     * StencilRead/WriteMask: 예) uint8 중 어떤 비트를 사용할지
     */

    /* D3D11_DEPTH_STENCILOP_DESC 옵션 정리
     * https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencilop_desc
     * StencilPassOp : 둘 다 pass일 때 할 일
     * StencilDepthFailOp : Stencil pass, Depth fail 일 때 할 일
     * StencilFailOp : 둘 다 fail 일 때 할 일
     */

    // m_drawDSS: 지금까지 사용해온 기본 DSS
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc;
        ZeroMemory(&dsDesc, sizeof(dsDesc));
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = false; // Stencil 불필요
        dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        // 앞면에 대해서 어떻게 작동할지 설정
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        // 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
        dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        ThrowIfFailed(m_device->CreateDepthStencilState(
            &dsDesc, m_drawDSS.GetAddressOf()));

        // Stencil에 1로 표기해주는 DSS
        dsDesc.DepthEnable = true; // 이미 그려진 물체 유지
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = true;    // Stencil 필수
        dsDesc.StencilReadMask = 0xFF;  // 모든 비트 다 사용
        dsDesc.StencilWriteMask = 0xFF; // 모든 비트 다 사용
        // 앞면에 대해서 어떻게 작동할지 설정
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        ThrowIfFailed(m_device->CreateDepthStencilState(
            &dsDesc, m_maskDSS.GetAddressOf()));

        // Stencil에 1로 표기된 경우에"만" 그리는 DSS
        // DepthBuffer는 초기화된 상태로 가정
        // D3D11_COMPARISON_EQUAL 이미 1로 표기된 경우에만 그리기
        // OMSetDepthStencilState(..., 1); <- 여기의 1
        dsDesc.DepthEnable = true;   // 거울 속을 다시 그릴때 필요
        dsDesc.StencilEnable = true; // Stencil 사용
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // <- 주의
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

        ThrowIfFailed(m_device->CreateDepthStencilState(
            &dsDesc, m_drawMaskedDSS.GetAddressOf()));
    }

    /* "이미 그려져있는 화면"과 어떻게 섞을지를 결정
     * Dest: 이미 그려져 있는 값들을 의미
     * Src: 픽셀 쉐이더가 계산한 값들을 의미 (여기서는 마지막 거울)
     */
    {
        D3D11_BLEND_DESC mirrorBlendDesc;
        ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
        mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
        mirrorBlendDesc.IndependentBlendEnable = false;
        // 개별 RenderTarget에 대해서 설정 (최대 8개)
        mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
        mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
        mirrorBlendDesc.RenderTarget[0].DestBlend =
            D3D11_BLEND_INV_BLEND_FACTOR;
        mirrorBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

        mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        // 필요하면 RGBA 각각에 대해서도 조절 가능
        mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
            D3D11_COLOR_WRITE_ENABLE_ALL;

        ThrowIfFailed(m_device->CreateBlendState(&mirrorBlendDesc,
                                                 m_mirrorBS.GetAddressOf()));
    }
}

bool AppBase::InitMainWindow() {

    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"HongLabGraphics", // lpszClassName, L-string
                     NULL};

    if (!RegisterClassEx(&wc)) {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    RECT wr = {0, 0, m_screenWidth, m_screenHeight};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
    m_mainWindow = CreateWindow(wc.lpszClassName, L"HongLabGraphics Example",
                                WS_OVERLAPPEDWINDOW,
                                100, // 윈도우 좌측 상단의 x 좌표
                                100, // 윈도우 좌측 상단의 y 좌표
                                wr.right - wr.left, // 윈도우 가로 방향 해상도
                                wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                                NULL, NULL, wc.hInstance, NULL);

    if (!m_mainWindow) {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_mainWindow);

    return true;
}

bool AppBase::InitDirect3D() {

    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_mainWindow;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(), &featureLevel, m_context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        cout << "D3D Feature Level 11 unsupported." << endl;
        return false;
    }

    CheckMSAA();

    CreateBuffers();

    SetViewport();

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;
    rastDesc.MultisampleEnable = true;

    ThrowIfFailed(
        m_device->CreateRasterizerState(&rastDesc, m_solidRS.GetAddressOf()));

    // 거울에 반사되면 삼각형의 Winding이 바뀌기 때문에 CCW로 그려야함
    rastDesc.FrontCounterClockwise = true;
    ThrowIfFailed(m_device->CreateRasterizerState(&rastDesc,
                                                  m_solidCCWRS.GetAddressOf()));

    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
    ThrowIfFailed(
        m_device->CreateRasterizerState(&rastDesc, m_wireCCWRS.GetAddressOf()));

    rastDesc.FrontCounterClockwise = false;
    ThrowIfFailed(
        m_device->CreateRasterizerState(&rastDesc, m_wireRS.GetAddressOf()));

    // 공통으로 쓰이는 ConstBuffers
    D3D11Utils::CreateConstBuffer(m_device, m_eyeViewProjConstData,
                                  m_eyeViewProjConstBuffer);
    D3D11Utils::CreateConstBuffer(m_device, m_mirrorEyeViewProjConstData,
                                  m_mirrorEyeViewProjConstBuffer);
    D3D11Utils::CreateConstBuffer(m_device, m_depthEyeViewProjConstData,
                                  m_depthEyeViewProjConstBuffer);
    D3D11Utils::CreateConstBuffer(m_device, m_lightEyeViewProjConstData,
                                  m_lightEyeViewProjConstBuffer);

    return true;
}

bool AppBase::InitGUI() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_mainWindow)) {
        return false;
    }

    return true;
}

void AppBase::SetViewport() {

    static int previousGuiWidth = -1;

    if (previousGuiWidth != m_guiWidth) {

        previousGuiWidth = m_guiWidth;

        // Set the viewport
        ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
        m_screenViewport.TopLeftX = float(m_guiWidth);
        m_screenViewport.TopLeftY = 0;
        m_screenViewport.Width = float(m_screenWidth - m_guiWidth);
        m_screenViewport.Height = float(m_screenHeight);
        m_screenViewport.MinDepth = 0.0f;
        m_screenViewport.MaxDepth = 1.0f;
    }

    m_context->RSSetViewports(1, &m_screenViewport);
}

bool CheckFormats(ComPtr<ID3D11Device> &device,
                  const std::vector<DXGI_FORMAT> &formatsToUse,
                  const UINT sampleCount, UINT &maxNumQualityLevels) {

    maxNumQualityLevels = 0;
    for (auto format : formatsToUse) {
        UINT temp = 0;
        HRESULT hr =
            device->CheckMultisampleQualityLevels(format, sampleCount, &temp);

        // CheckMQLs(.) 통과 조건은 Remakrs 참고
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-checkmultisamplequalitylevels
        if (SUCCEEDED(hr) && temp > 0) {
            maxNumQualityLevels = std::max(maxNumQualityLevels, temp);
        } else {
            return false;
        }
    }

    return true;
}

void AppBase::CheckMSAA() {
    if (m_useMSAA == false) {
        m_numQualityLevels = 1;
        m_sampleCount = 1;
    } else {
        std::vector<DXGI_FORMAT> formatsToUse = {
            DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R16G16B16A16_FLOAT,
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16_FLOAT};

        for (UINT sampleCount = 1; sampleCount <= m_targetSampleCount;
             sampleCount *= 2) {

            UINT maxNumQualityLevels = 0;
            if (CheckFormats(m_device, formatsToUse, sampleCount,
                             maxNumQualityLevels)) {
                m_sampleCount = sampleCount;
                m_numQualityLevels = maxNumQualityLevels;
            } else {
                break;
            }
        }
    }

    std::cout << "CheckMSAA() : " << m_sampleCount << " " << m_numQualityLevels
              << std::endl;
}

void AppBase::CreateBuffers() {

    // 레스터화 -> float/depthBuffer(MSAA) -> resolved -> backBuffer

    // BackBuffer는 화면으로 최종 출력 (SRV는 불필요)
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        backBuffer.Get(), NULL, m_backBufferRTV.GetAddressOf()));

    m_mainBuffer.Initialize(m_device, m_screenWidth, m_screenHeight,
                            DXGI_FORMAT_R16G16B16A16_FLOAT, m_numQualityLevels,
                            m_sampleCount);

    // Depth만 렌더링하는 경우에는 R16_FLOAT로도 가능
    m_shadowBuffer.Initialize(m_device, m_shadowWidth, m_shadowHeight,
                              DXGI_FORMAT_R16_FLOAT,
                              m_numQualityLevels, m_sampleCount);

    CreateDepthBuffers();

    m_postProcess.Initialize(m_device, m_context, {m_mainBuffer.m_resolvedSRV},
                             {m_backBufferRTV}, m_screenWidth, m_screenHeight,
                             4);
}

} // namespace hlab