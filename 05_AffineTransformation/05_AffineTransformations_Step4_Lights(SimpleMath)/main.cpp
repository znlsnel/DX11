#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <memory>
#include <windows.h>

#include "Example.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 콘솔창이 있으면 디버깅에 편리합니다.
// 디버깅할 때 애매한 값들을 cout으로 출력해서 확인해보세요.
int main() {
    // 내 모니터에 맞는 해상도로 조절하셔도 됩니다.
    const int width = 1280, height = 960;

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

    RegisterClassEx(&wc);

    // 툴바까지 포함한 윈도우 전체 해상도가 아니라
    // 우리가 실제로 그리는 해상도가 width x height가 되도록
    // 윈도우를 만들 해상도를 다시 계산해서 CreateWindow()에서 사용

    // 우리가 원하는 그림이 그려질 부분의 해상도
    RECT wr = {0, 0, width, height};

    // 필요한 윈도우 크기(해상도) 계산
    // wr의 값이 바뀜
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // 윈도우를 만들때 위에서 계산한 wr 사용
    HWND hwnd = CreateWindow(wc.lpszClassName, L"HongLabGraphics Example",
                             WS_OVERLAPPEDWINDOW,
                             100, // 윈도우 좌측 상단의 x 좌표
                             100, // 윈도우 좌측 상단의 y 좌표
                             wr.right - wr.left, // 윈도우 가로 방향 해상도
                             wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                             NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // 예제 클래스의 인스턴스 생성 (unique_ptr로 만듦)
    // 비교: example = new Example(...)
    auto example = std::make_unique<hlab::Example>(hwnd, width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(width, height);
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplDX11_Init(example->device, example->deviceContext);
    ImGui_ImplWin32_Init(hwnd);

    // Main message loop
    MSG msg = {};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Start the Dear ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            // ImGui::Checkbox("cullBackface",
            //                 &example->rasterization.cullBackface);

            ImGui::SliderAngle(
                "Object RotationAboutX",
                &example->rasterization.object->transformation.rotationX);

            ImGui::SliderAngle(
                "Object RotationAboutY",
                &example->rasterization.object->transformation.rotationY);

            ImGui::SliderFloat3(
                "Object Translation",
                &example->rasterization.object->transformation.translation.x,
                -3.0f, 4.0f);

            ImGui::SliderFloat3(
                "Object Scale",
                &example->rasterization.object->transformation.scale.x, 0.1f,
                2.0f);

            ImGui::SliderFloat3(
                "Material ambient",
                &example->rasterization.object->material.ambient.x, 0.0f, 1.0f);

            ImGui::SliderFloat3(
                "Material diffuse",
                &example->rasterization.object->material.diffuse.x, 0.0f, 1.0f);

            ImGui::SliderFloat3(
                "Material specular",
                &example->rasterization.object->material.specular.x, 0.0f,
                1.0f);

            ImGui::SliderFloat(
                "Material shininess",
                &example->rasterization.object->material.shininess, 0.0f,
                256.0f);

            if (ImGui::RadioButton("Directional Light",
                                   example->rasterization.lightType == 0)) {
                example->rasterization.lightType = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Point Light",
                                   example->rasterization.lightType == 1)) {
                example->rasterization.lightType = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Spot Light",
                                   example->rasterization.lightType == 2)) {
                example->rasterization.lightType = 2;
            }

            ImGui::SliderFloat3("Light Strength",
                                &example->rasterization.light.strength.x, 0.0f,
                                1.0f);

            if (ImGui::SliderFloat3("Light Direction",
                                    &example->rasterization.light.direction.x,
                                    -3.0f, 3.0f)) {
                if (example->rasterization.light.direction.Length() > 1e-5f) {
                    example->rasterization.light.direction.Normalize();
                }
            };

            ImGui::SliderFloat3("Light Position",
                                &example->rasterization.light.position.x, -2.0f,
                                2.0f);

            ImGui::SliderFloat("Light fallOffStart",
                               &example->rasterization.light.fallOffStart, 0.0f,
                               5.0f);

            ImGui::SliderFloat("Light fallOffEnd",
                               &example->rasterization.light.fallOffEnd, 0.0f,
                               10.0f);

            ImGui::SliderFloat("Light spotPower",
                               &example->rasterization.light.spotPower, 0.0f,
                               512.0f);

            ImGui::End();
            ImGui::Render();

            example->Update();
            example->Render();

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // switch the back buffer and the front buffer
            example->swapChain->Present(1, 0);
        }
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    example->Clean();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

// Windows procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        // Reset and resize swapchain
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE:
        // std::cout << "Mouse " << LOWORD(lParam) << " " << HIWORD(lParam) <<
        // std::endl;
        break;
    case WM_LBUTTONUP:
        // std::cout << "WM_LBUTTONUP Left mouse button" << std::endl;
        break;
    case WM_RBUTTONUP:
        // std::cout << "WM_RBUTTONUP Right mouse button" << std::endl;
        break;
    case WM_KEYDOWN:
        // std::cout << "WM_KEYDOWN " << (int)wParam << std::endl;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
