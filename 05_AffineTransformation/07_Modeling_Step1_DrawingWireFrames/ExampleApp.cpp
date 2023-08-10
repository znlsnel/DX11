#include "ExampleApp.h"

#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

namespace hlab {

using namespace std;

ExampleApp::ExampleApp()
    : AppBase(), m_indexCount(0), m_pixelConstantBufferData() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    // Texture 만들기
    // https://opengameart.org/content/3-crate-textures-w-bump-normal
    // https://learnopengl.com/Getting-started/Textures
    AppBase::CreateTexture("crate2_diffuse.png", m_texture,
                           m_textureResourceView);

    AppBase::CreateTexture("wall.jpg", m_texture2, m_textureResourceView2);

    // Texture sampler 만들기
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the Sample State
    m_device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

    // Geometry 정의
    MeshData meshData = GeometryGenerator::MakeBox();

    // 버텍스 버퍼 만들기
    AppBase::CreateVertexBuffer(meshData.vertices, m_vertexBuffer);

    // 인덱스 버퍼 만들기
    m_indexCount = UINT(meshData.indices.size());

    AppBase::CreateIndexBuffer(meshData.indices, m_indexBuffer);

    // ConstantBuffer 만들기
    m_vertexConstantBufferData.model = Matrix();
    m_vertexConstantBufferData.view = Matrix();
    m_vertexConstantBufferData.projection = Matrix();

    AppBase::CreateConstantBuffer(m_vertexConstantBufferData,
                                  m_vertexConstantBuffer);

    AppBase::CreateConstantBuffer(m_pixelConstantBufferData,
                                  m_pixelShaderConstantBuffer);

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
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    AppBase::CreateVertexShaderAndInputLayout(
        L"BasicVertexShader.hlsl", inputElements, m_colorVertexShader,
        m_colorInputLayout);

    AppBase::CreatePixelShader(L"BasicPixelShader.hlsl", m_colorPixelShader);

    return true;
}

void ExampleApp::Update(float dt) {

    using namespace DirectX;

    // 모델의 변환
    m_vertexConstantBufferData.model =
        Matrix::CreateScale(m_modelScaling) *
        Matrix::CreateRotationY(m_modelRotation.y) *
        Matrix::CreateRotationX(m_modelRotation.x) *
        Matrix::CreateRotationZ(m_modelRotation.z) *
        Matrix::CreateTranslation(m_modelTranslation);
    m_vertexConstantBufferData.model =
        m_vertexConstantBufferData.model.Transpose();

    m_vertexConstantBufferData.invTranspose = m_vertexConstantBufferData.model;
    m_vertexConstantBufferData.invTranspose.Translation(Vector3(0.0f));
    m_vertexConstantBufferData.invTranspose =
        m_vertexConstantBufferData.invTranspose.Transpose().Invert();

    // 시점 변환
    m_vertexConstantBufferData.view =
        Matrix::CreateRotationY(m_viewRot) *
        Matrix::CreateTranslation(0.0f, 0.0f, 2.0f);

    m_pixelConstantBufferData.eyeWorld = Vector3::Transform(
        Vector3(0.0f), m_vertexConstantBufferData.view.Invert());

    m_vertexConstantBufferData.view =
        m_vertexConstantBufferData.view.Transpose();

    // 프로젝션
    const float aspect = AppBase::GetAspectRatio(); // <- GUI에서 조절
    if (m_usePerspectiveProjection) {
        m_vertexConstantBufferData.projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(m_projFovAngleY), aspect, m_nearZ, m_farZ);
    } else {
        m_vertexConstantBufferData.projection = XMMatrixOrthographicOffCenterLH(
            -aspect, aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
    }
    m_vertexConstantBufferData.projection =
        m_vertexConstantBufferData.projection.Transpose();

    // Constant를 CPU에서 GPU로 복사
    AppBase::UpdateBuffer(m_vertexConstantBufferData, m_vertexConstantBuffer);

    m_pixelConstantBufferData.material.diffuse = Vector3(m_materialDiffuse);
    m_pixelConstantBufferData.material.specular = Vector3(m_materialSpecular);

    // 여러 개 조명 사용 예시
    for (int i = 0; i < MAX_LIGHTS; i++) {
        // 다른 조명 끄기
        if (i != m_lightType) {
            m_pixelConstantBufferData.lights[i].strength *= 0.0f;
        } else {
            m_pixelConstantBufferData.lights[i] = m_lightFromGUI;
        }
    }

    AppBase::UpdateBuffer(m_pixelConstantBufferData,
                          m_pixelShaderConstantBuffer);
}

void ExampleApp::Render() {

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    // m_context->RSSetViewports(1, &m_screenViewport);
    SetViewport();

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
        m_vertexConstantBuffer.Get(),
    };
    m_context->VSSetConstantBuffers(0, 1, pptr); */

    m_context->VSSetConstantBuffers(0, 1,
                                    m_vertexConstantBuffer.GetAddressOf());

    ID3D11ShaderResourceView *pixelResources[2] = {
        m_textureResourceView.Get(), m_textureResourceView2.Get()};
    m_context->PSSetShaderResources(0, 2, pixelResources);
    m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    m_context->PSSetConstantBuffers(0, 1,
                                    m_pixelShaderConstantBuffer.GetAddressOf());
    m_context->PSSetShader(m_colorPixelShader.Get(), 0, 0);

    // TODO:
    m_context->RSSetState(m_solidRasterizerSate.Get());

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

    ImGui::Checkbox("Use Texture", &m_pixelConstantBufferData.useTexture);

    ImGui::Checkbox("Wireframe", &m_drawAsWire);

    ImGui::SliderFloat3("m_modelTranslation", &m_modelTranslation.x, -2.0f,
                        2.0f);
    ImGui::SliderFloat3("m_modelRotation", &m_modelRotation.x, -3.14f, 3.14f);
    ImGui::SliderFloat3("m_modelScaling", &m_modelScaling.x, 0.1f, 2.0f);
    ImGui::SliderFloat("m_viewRot", &m_viewRot, -3.14f, 3.14f);

    ImGui::SliderFloat("Material Shininess",
                       &m_pixelConstantBufferData.material.shininess, 1.0f,
                       256.0f);

    if (ImGui::RadioButton("Directional Light", m_lightType == 0)) {
        m_lightType = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Point Light", m_lightType == 1)) {
        m_lightType = 1;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Spot Light", m_lightType == 2)) {
        m_lightType = 2;
    }

    ImGui::SliderFloat("Material Diffuse", &m_materialDiffuse, 0.0f, 1.0f);
    ImGui::SliderFloat("Material Specular", &m_materialSpecular, 0.0f, 1.0f);

    ImGui::SliderFloat3("Light Position", &m_lightFromGUI.position.x, -5.0f,
                        5.0f);

    ImGui::SliderFloat("Light fallOffStart", &m_lightFromGUI.fallOffStart, 0.0f,
                       5.0f);

    ImGui::SliderFloat("Light fallOffEnd", &m_lightFromGUI.fallOffEnd, 0.0f,
                       10.0f);

    ImGui::SliderFloat("Light spotPower", &m_lightFromGUI.spotPower, 1.0f,
                       512.0f);
}

} // namespace hlab
