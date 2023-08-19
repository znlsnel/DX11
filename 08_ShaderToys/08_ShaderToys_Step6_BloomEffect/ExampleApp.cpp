﻿#include "ExampleApp.h"

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

    m_cubeMapping.Initialize(m_device,
                             L"./CubemapTextures/Stonewall_diffuseIBL.dds",
                             L"./CubemapTextures/Stonewall_specularIBL.dds");

    MeshData sphere = GeometryGenerator::MakeSphere(0.3f, 100, 100);
    sphere.textureFilename = "ojwD8.jpg";
    m_meshGroupSphere.Initialize(m_device, {sphere});
    m_meshGroupSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    m_meshGroupSphere.m_specularResView = m_cubeMapping.m_specularResView;

    m_meshGroupCharacter.Initialize(m_device, "C:/DEVELOPMENT/GIT/DX11_HongLab/models/glTF-Sample-Models-master/2.0/Lantern/glTF/", "Lantern.gltf");
    m_meshGroupCharacter.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    m_meshGroupCharacter.m_specularResView = m_cubeMapping.m_specularResView;

    BuildFilters();

    return true;
}

void ExampleApp::Update(float dt) {

    using namespace DirectX;

    auto &visibleMeshGroup =
        m_visibleMeshIndex == 0 ? m_meshGroupSphere : m_meshGroupCharacter;

    auto modelRow = Matrix::CreateScale(m_modelScaling) *
                    Matrix::CreateRotationY(m_modelRotation.y) *
                    Matrix::CreateRotationX(m_modelRotation.x) *
                    Matrix::CreateRotationZ(m_modelRotation.z) *
                    Matrix::CreateTranslation(m_modelTranslation);

    auto invTransposeRow = modelRow;
    invTransposeRow.Translation(Vector3(0.0f));
    invTransposeRow = invTransposeRow.Invert().Transpose();

    auto viewRow = Matrix::CreateRotationY(m_viewRot.y) *
                   Matrix::CreateRotationX(m_viewRot.x) *
                   Matrix::CreateTranslation(0.0f, 0.0f, 2.0f);

    const float aspect = AppBase::GetAspectRatio();
    Matrix projRow =
        m_usePerspectiveProjection
            ? XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
                                       aspect, m_nearZ, m_farZ)
            : XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f, 1.0f,
                                              m_nearZ, m_farZ);

    auto eyeWorld = Vector3::Transform(Vector3(0.0f), viewRow.Invert());

    // MeshGroup의 ConstantBuffers 업데이트

    for (int i = 0; i < MAX_LIGHTS; i++) {
        // 다른 조명 끄기
        if (i != m_lightType) {
            visibleMeshGroup.m_basicPixelConstantData.lights[i].strength *=
                0.0f;
        } else {
            visibleMeshGroup.m_basicPixelConstantData.lights[i] =
                m_lightFromGUI;
        }
    }

    visibleMeshGroup.m_basicVertexConstantData.model = modelRow.Transpose();
    visibleMeshGroup.m_basicVertexConstantData.view = viewRow.Transpose();
    visibleMeshGroup.m_basicVertexConstantData.projection = projRow.Transpose();

    visibleMeshGroup.m_basicPixelConstantData.eyeWorld = eyeWorld;



    visibleMeshGroup.m_basicPixelConstantData.material.diffuse =
        Vector3(m_materialDiffuse);
    visibleMeshGroup.m_basicPixelConstantData.material.specular =
        Vector3(m_materialSpecular);
    visibleMeshGroup.UpdateConstantBuffers(m_device, m_context);

    // 큐브 매핑 Constant Buffer 업데이트
    m_cubeMapping.UpdateConstantBuffers(m_device, m_context,
                                        (Matrix::CreateRotationY(m_viewRot.y) *
                                         Matrix::CreateRotationX(m_viewRot.x))
                                            .Transpose(),
                                        projRow.Transpose());




    if (m_dirtyflag) {

        m_filters[1]->m_pixelConstData.dx = 1.0f / m_screenWidth;
        m_filters[1]->m_pixelConstData.dy = 1.0f / m_screenHeight;
        m_filters[1]->m_pixelConstData.threshold = m_threshold;
        m_filters.back()->m_pixelConstData.strength = m_strength;
        m_filters[1]->UpdateConstantBuffers(m_device, m_context);
        m_filters.back()->UpdateConstantBuffers(m_device, m_context);


        m_dirtyflag = 0;
    }
}

void ExampleApp::Render() {

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    SetViewport();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
                                  m_depthStencilView.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    if (m_drawAsWire) {
        m_context->RSSetState(m_wireRasterizerSate.Get());
    } else {
        m_context->RSSetState(m_rasterizerSate.Get());
    }

    // 큐브매핑
    m_cubeMapping.Render(m_context);


    // 물체들
    if (m_visibleMeshIndex == 0) {
        m_meshGroupSphere.Render(m_context);
    } else {
        m_meshGroupCharacter.Render(m_context);
    }

    // 후처리 필터
    for (auto &f : m_filters) {

        f->Render(m_context);
    }
}

void ExampleApp::BuildFilters() {


    m_filters.clear();


    auto copyFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
                                 m_screenWidth, m_screenHeight);
    copyFilter->SetShaderResources({m_shaderResourceView});
    m_filters.push_back(copyFilter);

        auto downFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling", m_screenWidth / m_down,
        m_screenHeight / m_down);
    downFilter->SetShaderResources({m_shaderResourceView});
    downFilter->m_pixelConstData.threshold = m_threshold;
        downFilter->UpdateConstantBuffers(m_device, m_context);
        m_filters.push_back(downFilter);

      

        for (int down = m_down; down >= 1; down /= 2)
          for (int i =0; i < 5; i++){


              auto &preResource =  m_filters.back()->m_shaderResourceView;
                m_filters.push_back(make_shared<ImageFilter>(
                  m_device, m_context, L"Sampling", L"BlurX",
                  m_screenWidth / down, m_screenHeight / down));
              m_filters.back()->SetShaderResources({preResource});

                auto &preResource2 = m_filters.back()->m_shaderResourceView;
              m_filters.push_back(make_shared<ImageFilter>(
                    m_device, m_context, L"Sampling", L"BlurY",
                    m_screenWidth / down, m_screenHeight / down));
              m_filters.back()->SetShaderResources({preResource2});
   
                if (down > 1) {
                          auto &upFilter = m_filters.back()->m_shaderResourceView;
                          m_filters.push_back(make_shared<ImageFilter>(
                              m_device, m_context, L"Sampling", L"BlurY",
                              m_screenWidth / down * 2, m_screenHeight / down * 2));
                          m_filters.back()->SetShaderResources({upFilter});
                }
            }



        auto finalBlurFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Blur",
                                 m_screenWidth, m_screenHeight);
    // copyFilter의 출력이 FinalFilter의 입력으로 들어가는거
    finalBlurFilter->SetShaderResources(
            {copyFilter->m_shaderResourceView,
             m_filters.back()->m_shaderResourceView}
    );
        finalBlurFilter->SetRenderTargets({this->m_renderTargetView});
        finalBlurFilter->m_pixelConstData.strength = m_strength;
        finalBlurFilter->UpdateConstantBuffers(m_device, m_context);
    m_filters.push_back(finalBlurFilter);

}

void ExampleApp::UpdateGUI() {

    auto &meshGroup =
        m_visibleMeshIndex == 0 ? m_meshGroupSphere : m_meshGroupCharacter;

    if (ImGui::RadioButton("Sphere", m_visibleMeshIndex == 0)) {
        m_visibleMeshIndex = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Character", m_visibleMeshIndex == 1)) {
        m_visibleMeshIndex = 1;
    }

    m_dirtyflag +=
        ImGui::SliderFloat("Bloom Threshold", &m_threshold, 0.0f, 1.0f);
    m_dirtyflag +=
        ImGui::SliderFloat("Bloom Strength", &m_strength, 0.0f, 3.0f);

    ImGui::Checkbox("Use Texture",
                    &meshGroup.m_basicPixelConstantData.useTexture);
    ImGui::Checkbox("Wireframe", &m_drawAsWire);
    ImGui::Checkbox("Draw Normals", &meshGroup.m_drawNormals);
    if (ImGui::SliderFloat("Normal scale",
                           &meshGroup.m_normalVertexConstantData.scale, 0.0f,
                           1.0f)) {
        meshGroup.m_drawNormalsDirtyFlag = true;
    }

    ImGui::SliderFloat3("m_modelRotation", &m_modelRotation.x, -3.14f, 3.14f);
    ImGui::SliderFloat3("m_viewRot", &m_viewRot.x, -3.14f, 3.14f);
    ImGui::SliderFloat3(
        "Material FresnelR0",
        &meshGroup.m_basicPixelConstantData.material.fresnelR0.x, 0.0f, 1.0f);

    ImGui::SliderFloat("Material Diffuse", &m_materialDiffuse, 0.0f, 3.0f);
    ImGui::SliderFloat("Material Specular", &m_materialSpecular, 0.0f, 3.0f);
    ImGui::SliderFloat("Material Shininess",
                       &meshGroup.m_basicPixelConstantData.material.shininess,
                       0.01f, 20.0f);
}

} // namespace hlab
