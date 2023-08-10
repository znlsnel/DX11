#pragma once

#include "Rasterization.h"

#include <algorithm>
#include <chrono>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <vector>
#include <windows.h>

namespace hlab {
using namespace glm;
using namespace std;

struct Vertex {
    glm::vec4 pos;
    glm::vec2 uv;
};

class Example {

    // 구글 코딩 스타일의 선언 순서(Declaration Order)를 따라서
    // 데이터 멤버(멤버 변수)들을 멤버 함수들 아래에 선언하였습니다.
    // https://google.github.io/styleguide/cppguide.html#Declaration_Order

    // 여기서는 가급적 코드를 단순하게 만들기 위해서
    // 모든 멤버 함수/변수들을 public으로 선언하였습니다.
    // 실무용 코드에서는 객체지향의 캡슐화 원칙을 따르기 위해서
    // 데이터 멤버들은 private으로 선언하는 경우가 많습니다.
    // 변수 이름에서 멤버라는 의미로 앞에 붙이는 m_ 생략했습니다.
    // 예) m_width -> width

  public:
    Example(HWND window, int width, int height);

    // 메인 루프에서 Update()와 Render()를 반복해서 호출합니다.
    void Update();
    void Render();

    void InitShaders();
    void Initialize(HWND window, int width, int height);
    void Clean();

  public:
    int width, height; // 윈도우에서 그림이 그려지는 화면의 해상도
    std::vector<vec4> pixels;    // 색깔 값들 저장
    Rasterization rasterization; // Render()에서 pixels에 그림

    ID3D11Device *device;
    ID3D11DeviceContext *deviceContext;
    IDXGISwapChain *swapChain;
    D3D11_VIEWPORT viewport;
    ID3D11RenderTargetView *renderTargetView;
    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *pixelShader;
    ID3D11InputLayout *layout;

    ID3D11Buffer *vertexBuffer = nullptr;
    ID3D11Buffer *indexBuffer = nullptr;
    ID3D11Texture2D *canvasTexture = nullptr;
    ID3D11ShaderResourceView *canvasTextureView = nullptr;
    ID3D11RenderTargetView *canvasRenderTargetView = nullptr;
    ID3D11SamplerState *colorSampler;
    UINT indexCount;
};
} // namespace hlab