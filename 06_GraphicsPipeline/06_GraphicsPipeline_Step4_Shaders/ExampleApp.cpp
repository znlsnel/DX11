#include "ExampleApp.h"

#include <tuple>
#include <vector>

namespace hlab {

using namespace std;

auto MakeSquare() {
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표

    const float scale = 1.0f;

    // 앞면
    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // TODO: 텍스춰 좌표 추가
    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates

    vector<Vertex> vertices;
    for (size_t i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.color = colors[i];
        // TODO: 텍스춰 좌표 추가
        vertices.push_back(v);
    }
    vector<uint16_t> indices = {
        0, 1, 2, 0, 2, 3, // 앞면
    };

    return tuple{vertices, indices};
}

auto MakeBox() {

    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;

    const float scale = 1.0f;

    // 윗면
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

    // 아랫면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));

    // 앞면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // 뒷면
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));

    // 왼쪽
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));

    // 오른쪽
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));

    vector<Vertex> vertices;
    for (size_t i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.color = colors[i];
        vertices.push_back(v);
    }

    vector<uint16_t> indices = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };

    return tuple{vertices, indices};
}

ExampleApp::ExampleApp() : AppBase(), m_indexCount(0) {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    // Geometry 정의
    auto [vertices, indices] = MakeSquare();

    // 버텍스 버퍼 만들기
    AppBase::CreateVertexBuffer(vertices, m_vertexBuffer);

    // 인덱스 버퍼 만들기
    m_indexCount = UINT(indices.size());

    AppBase::CreateIndexBuffer(indices, m_indexBuffer);

    // ConstantBuffer 만들기
    m_constantBufferData.model = Matrix();
    m_constantBufferData.view = Matrix();
    m_constantBufferData.projection = Matrix();

    AppBase::CreateConstantBuffer(m_constantBufferData, m_constantBuffer);

    // TODO: 픽셀쉐이더로 보낼 ConstantBuffer 만들기

    // 쉐이더 만들기

    // Input-layout objects describe how vertex buffer data is streamed into the
    // IA(Input-Assembler) pipeline stage.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-iasetinputlayout

    // Input-Assembler Stage
    // The purpose of the input-assembler stage is to read primitive data
    // (points, lines and/or triangles) from user-filled buffers and assemble
    // the data into primitives that will be used by the other pipeline stages.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-input-assembler-stage

    // D3D11_INPUT_ELEMENT_DESC structure(d3d11.h)
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_input_element_desc

    // Semantics
    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics

    vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        // TODO: 텍스춰 좌표를 버텍스 쉐이더로 보내겠다!
    };

    AppBase::CreateVertexShaderAndInputLayout(
        L"ColorVertexShader.hlsl", inputElements, m_colorVertexShader,
        m_colorInputLayout);

    AppBase::CreatePixelShader(L"ColorPixelShader.hlsl", m_colorPixelShader);

    return true;
}

void ExampleApp::Update(float dt) {

    using namespace DirectX;

    // 모델의 변환
    m_constantBufferData.model =
        Matrix::CreateScale(1.5f) *
        Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 1.0f));
    m_constantBufferData.model = m_constantBufferData.model.Transpose();

    // 시점 변환
    // m_constantBufferData.view = XMMatrixLookAtLH(m_viewEye, m_viewFocus,
    // m_viewUp);
    m_constantBufferData.view =
        XMMatrixLookToLH(m_viewEyePos, m_viewEyeDir, m_viewUp);
    m_constantBufferData.view = m_constantBufferData.view.Transpose();

    // 프로젝션
    // m_aspect = AppBase::GetAspectRatio(); // <- GUI에서 조절
    if (m_usePerspectiveProjection) {
        m_constantBufferData.projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(m_projFovAngleY), m_aspect, m_nearZ, m_farZ);
    } else {
        m_constantBufferData.projection = XMMatrixOrthographicOffCenterLH(
            -m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
    }
    m_constantBufferData.projection =
        m_constantBufferData.projection.Transpose();

    // Constant를 CPU에서 GPU로 복사
    AppBase::UpdateBuffer(m_constantBufferData, m_constantBuffer);

    // TODO: 픽셀 쉐이더에서 사용할 ConstantBuffer 업데이트
}

void ExampleApp::Render() {

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    m_context->RSSetViewports(1, &m_screenViewport);

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);

    // 비교: Depth Buffer를 사용하지 않는 경우
    // m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
    // nullptr);
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
                                  m_depthStencilView.Get());

    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    // 어떤 쉐이더를 사용할지 설정
    m_context->VSSetShader(m_colorVertexShader.Get(), 0, 0);

    /* 경우에 따라서는 포인터의 배열을 넣어줄 수도 있습니다.
    ID3D11Buffer *pptr[1] = {
        m_constantBuffer.Get(),
    };
    m_context->VSSetConstantBuffers(0, 1, pptr); */

    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // TODO: 여기서 뭘 해줘야 할까요?

    m_context->PSSetShader(m_colorPixelShader.Get(), 0, 0);

    m_context->RSSetState(m_rasterizerSate.Get());

    // 버텍스/인덱스 버퍼 설정
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetInputLayout(m_colorInputLayout.Get());
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride,
                                  &offset);
    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->DrawIndexed(m_indexCount, 0, 0);
}

void ExampleApp::UpdateGUI() {
    // TODO: GUI 기능 추가
    // ImGui::SliderFloat("xSplit", &m_pixelShaderConstantBufferData.xSplit,
    // 0.0f,
    //                   1.0f);
}

} // namespace hlab
