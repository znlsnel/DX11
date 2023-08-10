#include "ExampleApp.h"

#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

namespace hlab {

using namespace std;
using namespace DirectX;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    //m_cubeMapping.Initialize(m_device,
    //                         L"./CubemapTextures/Stonewall_diffuseIBL.dds",
    //                         L"./CubemapTextures/Stonewall_specularIBL.dds");

    //MeshData sphere = GeometryGenerator::MakeSphere(0.3f, 100, 100);
    //sphere.textureFilename = "ojwD8.jpg";
    //m_meshGroupSphere.Initialize(m_device, {sphere});
    //m_meshGroupSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    //m_meshGroupSphere.m_specularResView = m_cubeMapping.m_specularResView;

    //m_meshGroupCharacter.Initialize(m_device, "c:/zelda/", "zeldaPosed001.fbx");
    //m_meshGroupCharacter.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    //m_meshGroupCharacter.m_specularResView = m_cubeMapping.m_specularResView;

    BuildFilters();

    return true;
}

void ExampleApp::Update(float dt) {

    using namespace DirectX;

    //auto &visibleMeshGroup =
    //    m_visibleMeshIndex == 0 ? m_meshGroupSphere : m_meshGroupCharacter;

    //auto modelRow = Matrix::CreateScale(m_modelScaling) *
    //                Matrix::CreateRotationY(m_modelRotation.y) *
    //                Matrix::CreateRotationX(m_modelRotation.x) *
    //                Matrix::CreateRotationZ(m_modelRotation.z) *
    //                Matrix::CreateTranslation(m_modelTranslation);

    //auto invTransposeRow = modelRow;
    //invTransposeRow.Translation(Vector3(0.0f));
    //invTransposeRow = invTransposeRow.Invert().Transpose();

    //auto viewRow = Matrix::CreateRotationY(m_viewRot.y) *
    //               Matrix::CreateRotationX(m_viewRot.x) *
    //               Matrix::CreateTranslation(0.0f, 0.0f, 2.0f);

    //const float aspect = AppBase::GetAspectRatio();
    //Matrix projRow =
    //    m_usePerspectiveProjection
    //        ? XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
    //                                   aspect, m_nearZ, m_farZ)
    //        : XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f, 1.0f,
    //                                          m_nearZ, m_farZ);

    //auto eyeWorld = Vector3::Transform(Vector3(0.0f), viewRow.Invert());

    //// MeshGroup의 ConstantBuffers 업데이트

    //for (int i = 0; i < MAX_LIGHTS; i++) {
    //    // 다른 조명 끄기
    //    if (i != m_lightType) {
    //        visibleMeshGroup.m_basicPixelConstantData.lights[i].strength *=
    //            0.0f;
    //    } else {
    //        visibleMeshGroup.m_basicPixelConstantData.lights[i] =
    //            m_lightFromGUI;
    //    }
    //}

    //visibleMeshGroup.m_basicVertexConstantData.model = modelRow.Transpose();
    //visibleMeshGroup.m_basicVertexConstantData.view = viewRow.Transpose();
    //visibleMeshGroup.m_basicVertexConstantData.projection = projRow.Transpose();

    //visibleMeshGroup.m_basicPixelConstantData.eyeWorld = eyeWorld;

    //visibleMeshGroup.m_basicPixelConstantData.material.diffuse =
    //    Vector3(m_materialDiffuse);
    //visibleMeshGroup.m_basicPixelConstantData.material.specular =
    //    Vector3(m_materialSpecular);
    //visibleMeshGroup.UpdateConstantBuffers(m_device, m_context);

    // 큐브 매핑 Constant Buffer 업데이트
    //m_cubeMapping.UpdateConstantBuffers(m_device, m_context,
    //                                    (Matrix::CreateRotationY(m_viewRot.y) *
    //                                     Matrix::CreateRotationX(m_viewRot.x))
    //                                        .Transpose(),
    //                                    projRow.Transpose());

    static float time = 0.0f;

    m_filters.back()->m_pixelConstData.iTime = time;
    m_filters.back()->UpdateConstantBuffers(m_device, m_context);

    time += dt;
}

void ExampleApp::Render() {

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    //SetViewport();

    //float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    //m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    //m_context->ClearDepthStencilView(m_depthStencilView.Get(),
    //                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    //                                 1.0f, 0);
    //m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
    //                              m_depthStencilView.Get());
    //m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    //if (m_drawAsWire) {
    //    m_context->RSSetState(m_wireRasterizerSate.Get());
    //} else {
    //    m_context->RSSetState(m_rasterizerSate.Get());
    //}

    //// 큐브매핑
    //m_cubeMapping.Render(m_context);

    //// 물체들
    //if (m_visibleMeshIndex == 0) {
    //    m_meshGroupSphere.Render(m_context);
    //} else {
    //    m_meshGroupCharacter.Render(m_context);
    //}

    // 후처리 필터
    for (auto &f : m_filters) {
        f->Render(m_context);
    }
}

void ExampleApp::BuildFilters() {

    static ComPtr<ID3D11Texture2D> texture;
    static ComPtr<ID3D11ShaderResourceView> textureResourceView;

    D3D11Utils::CreateTexture(m_device, "shadertoytexture0.jpg", texture,
                              textureResourceView);

    m_filters.clear();

    //auto shaderToy =
    //    make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Seascape",
    //                             m_screenWidth, m_screenHeight);

    auto shaderToy =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Star",
                                 m_screenWidth, m_screenHeight);

    shaderToy->SetShaderResources({textureResourceView});
    shaderToy->SetRenderTargets({m_renderTargetView});
    m_filters.push_back(shaderToy);
}

void ExampleApp::UpdateGUI() {
}

} // namespace hlab
