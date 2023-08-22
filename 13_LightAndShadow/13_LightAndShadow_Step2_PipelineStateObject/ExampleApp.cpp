﻿#include "ExampleApp.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

    // 환경 박스 초기화
    {
        MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        m_skybox = make_shared<Model>(m_device, m_context, vector{skyboxMesh});
    }

    // 조명 설정
    {
        m_light.position = Vector3(0.0f, 0.5f, 1.7f);
        m_light.radiance = Vector3(5.0f);
        m_light.fallOffEnd = 20.0f;
    }

    // 거울
    {
        auto mesh = GeometryGenerator::MakeSquare(0.48f);
        m_mirror = make_shared<Model>(m_device, m_context, vector{mesh});
        m_mirror->m_materialConstsCPU.albedoFactor = Vector3(0.3f);
        m_mirror->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        m_mirror->m_materialConstsCPU.metallicFactor = 0.7f;
        m_mirror->m_materialConstsCPU.roughnessFactor = 0.2f;

        m_mirror->UpdateWorldRow(Matrix::CreateScale(1.0f, 1.5f, 1.0f) *
                                 Matrix::CreateRotationY(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(0.5f, 0.25f, 2.0f));

        m_mirrorPlane = SimpleMath::Plane(Vector3(0.5f, 0.25f, 2.0f),
                                          Vector3(-1.0f, 0.0f, 0.0f));
    }

    // 바닥
    {
        auto mesh = GeometryGenerator::MakeSquare(2.0);
        mesh.albedoTextureFilename =
            "../Assets/Textures/blender_uv_grid_2k.png";
        m_ground = make_shared<Model>(m_device, m_context, vector{mesh});
        m_ground->m_materialConstsCPU.albedoFactor = Vector3(0.2f);
        m_ground->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        m_ground->m_materialConstsCPU.metallicFactor = 0.2f;
        m_ground->m_materialConstsCPU.roughnessFactor = 0.8f;

        m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(0.0f, -0.5f, 2.0f));

        m_basicList.push_back(m_ground); // 리스트에 등록
    }

    // Main Object
    {
        // auto meshes = GeometryGenerator::ReadFromFile(
        //     "../Assets/Models/DamagedHelmet/", "DamagedHelmet.gltf");

        // auto meshes = GeometryGenerator::ReadFromFile(
        //     "../Assets/Models/medieval_vagrant_knights/", "scene.gltf");

        // 컴퓨터가 느릴 때는 간단한 물체로 테스트 하세요.
        // vector<MeshData> meshes = {GeometryGenerator::MakeBox(0.15f)};

        string path = "../Assets/Characters/armored-female-future-soldier/";
        auto meshes = GeometryGenerator::ReadFromFile(path, "angel_armor.fbx");
        meshes[0].albedoTextureFilename = path + "/angel_armor_albedo.jpg";
        meshes[0].emissiveTextureFilename = path + "/angel_armor_e.jpg";
        meshes[0].metallicTextureFilename = path + "/angel_armor_metalness.jpg";
        meshes[0].normalTextureFilename = path + "/angel_armor_normal.jpg";
        meshes[0].roughnessTextureFilename =
            path + "/angel_armor_roughness.jpg";

        Vector3 center(0.0f, 0.0f, 2.0f);
        m_mainObj = make_shared<Model>(m_device, m_context, meshes);
        m_mainObj->m_materialConstsCPU.invertNormalMapY =
            false; // GLTF는 true로
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(center));

        m_basicList.push_back(m_mainObj); // 리스트에 등록

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, 0.4f);
    }

    // 조명 위치 표시
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        m_lightSphere = make_shared<Model>(m_device, m_context, vector{sphere});
        m_lightSphere->UpdateWorldRow(
            Matrix::CreateTranslation(m_light.position));
        m_lightSphere->m_materialConstsCPU.albedoFactor = Vector3(0.0f);
        m_lightSphere->m_materialConstsCPU.emissionFactor =
            Vector3(1.0f, 1.0f, 0.0f);

        m_basicList.push_back(m_lightSphere); // 리스트에 등록
    }

    // 커서 표시 (Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구)
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        m_cursorSphere =
            make_shared<Model>(m_device, m_context, vector{sphere});
        m_cursorSphere->m_isVisible = false; // 마우스가 눌렸을 때만 보임
        m_cursorSphere->m_materialConstsCPU.albedoFactor = Vector3(0.0f);
        m_cursorSphere->m_materialConstsCPU.emissionFactor =
            Vector3(0.0f, 1.0f, 0.0f);

        m_basicList.push_back(m_cursorSphere); // 리스트에 등록
    }

    return true;
}

void ExampleApp::Update(float dt) {

    // 카메라의 이동
    m_camera.UpdateKeyboard(dt, m_keyPressed);

    // 반사 행렬 추가
    const Vector3 eyeWorld = m_camera.GetEyePos();
    const Matrix reflectRow = Matrix::CreateReflection(m_mirrorPlane);
    const Matrix viewRow = m_camera.GetViewRow();
    const Matrix projRow = m_camera.GetProjRow();

    // 조명 업데이트 (인덱스 1이 포인트라이트)
    AppBase::m_globalConstsCPU.lights[1] = m_light;
    AppBase::m_reflectGlobalConstsCPU.lights[1] = m_light;
    AppBase::UpdateGlobalConstants(eyeWorld, viewRow, projRow, reflectRow);

    // 거울은 따로 처리
    m_mirror->UpdateConstantBuffers(m_device, m_context);

    // 조명의 위치 반영
    m_lightSphere->UpdateWorldRow(Matrix::CreateTranslation(m_light.position));

    // 마우스 이동/회전 반영
    if (m_leftButton || m_rightButton) {
        Quaternion q;
        Vector3 dragTranslation;
        Vector3 pickPoint;
        if (UpdateMouseControl(m_mainBoundingSphere, q, dragTranslation,
                               pickPoint)) {
            Vector3 translation = m_mainObj->m_worldRow.Translation();
            m_mainObj->m_worldRow.Translation(Vector3(0.0f));
            m_mainObj->UpdateWorldRow(
                m_mainObj->m_worldRow * Matrix::CreateFromQuaternion(q) *
                Matrix::CreateTranslation(dragTranslation + translation));
            m_mainBoundingSphere.Center = m_mainObj->m_worldRow.Translation();

            // 충돌 지점에 작은 구 그리기
            m_cursorSphere->m_isVisible = true;
            m_cursorSphere->UpdateWorldRow(
                Matrix::CreateTranslation(pickPoint));
        } else {
            m_cursorSphere->m_isVisible = false;
        }
    }

    for (auto &i : m_basicList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }
}

void ExampleApp::Render() {

    AppBase::SetViewport();

    // 모든 샘플러들을 공통으로 사용 (뒤에서 더 추가됩니다.)
    m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());

    // 공용 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        m_envSRV.Get(), m_specularSRV.Get(), m_irradianceSRV.Get(),
        m_brdfSRV.Get()};
    m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    vector<ID3D11RenderTargetView *> rtvs = {m_floatRTV.Get()};
    for (size_t i = 0; i < rtvs.size(); i++) {
        m_context->ClearRenderTargetView(rtvs[i], clearColor);
    }

    m_context->OMSetRenderTargets(UINT(rtvs.size()), rtvs.data(),
                                  m_depthStencilView.Get());

    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);

    // 거울 1. 거울은 빼고 원래 대로 그리기
    AppBase::SetPipelineState(m_drawAsWire ? Graphics::defaultWirePSO
                                           : Graphics::defaultSolidPSO);
    AppBase::SetGlobalConsts(m_globalConstsGPU);

    for (auto &i : m_basicList) {
        i->Render(m_context);
    }

    AppBase::SetPipelineState(Graphics::normalsPSO);
    for (auto &i : m_basicList) {
        if (i->m_drawNormals)
            i->RenderNormals(m_context);
    }

    AppBase::SetPipelineState(m_drawAsWire ? Graphics::skyboxWirePSO
                                           : Graphics::skyboxSolidPSO);

    m_skybox->Render(m_context);

    // 거울 2. 거울 위치만 StencilBuffer에 1로 표기

    // 거울 3. 거울 위치에 반사된 물체들을 렌더링

    // 거울 4. 거울 자체의 재질을 "Blend"로 그림

    // 후처리
    m_context->ResolveSubresource(m_resolvedBuffer.Get(), 0,
                                  m_floatBuffer.Get(), 0,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);
    AppBase::SetPipelineState(Graphics::postProcessingPSO);
    m_postProcess.Render(m_context);
}

void ExampleApp::UpdateGUI() {

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_camera.m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
            CreateBuffers();
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Env Map")) {
        ImGui::SliderFloat("Strength", &m_globalConstsCPU.strengthIBL, 0.0f,
                           5.0f);
        ImGui::RadioButton("Env", &m_globalConstsCPU.textureToDraw, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Specular", &m_globalConstsCPU.textureToDraw, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Irradiance", &m_globalConstsCPU.textureToDraw, 2);
        ImGui::SliderFloat("EnvLodBias", &m_globalConstsCPU.envLodBias, 0.0f,
                           10.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Processing")) {
        int flag = 0;
        flag += ImGui::SliderFloat(
            "Bloom Strength",
            &m_postProcess.m_combineFilter.m_constData.strength, 0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Exposure", &m_postProcess.m_combineFilter.m_constData.option1,
            0.0f, 10.0f);
        flag += ImGui::SliderFloat(
            "Gamma", &m_postProcess.m_combineFilter.m_constData.option2, 0.1f,
            5.0f);
        // 편의상 사용자 입력이 인식되면 바로 GPU 버퍼를 업데이트
        if (flag) {
            m_postProcess.m_combineFilter.UpdateConstantBuffers(m_device,
                                                                m_context);
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Mirror")) {

        ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
        const float blendColor[4] = {m_mirrorAlpha, m_mirrorAlpha,
                                     m_mirrorAlpha, 1.0f};
        if (m_drawAsWire)
            Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
        else
            Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

        ImGui::SliderFloat("Metallic",
                           &m_mirror->m_materialConstsCPU.metallicFactor, 0.0f,
                           1.0f);
            ImGui::SliderFloat("Roughness",
                           &m_mirror->m_materialConstsCPU.roughnessFactor, 0.0f,
                           1.0f);

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Point Light")) {
        ImGui::SliderFloat3("Position", &m_light.position.x, -5.0f, 5.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material")) {
        ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias, 0.0f, 10.0f);

        int flag = 0;

        flag += ImGui::SliderFloat(
            "Metallic", &m_mainObj->m_materialConstsCPU.metallicFactor, 0.0f,
            1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &m_mainObj->m_materialConstsCPU.roughnessFactor, 0.0f,
            1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &m_mainObj->m_materialConstsCPU.useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "EmissiveTexture", &m_mainObj->m_materialConstsCPU.useEmissiveMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping", &m_mainObj->m_materialConstsCPU.useNormalMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &m_mainObj->m_materialConstsCPU.useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping", &m_mainObj->m_meshConstsCPU.useHeightMap, 1);
        flag += ImGui::SliderFloat(
            "HeightScale", &m_mainObj->m_meshConstsCPU.heightScale, 0.0f, 0.1f);
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap", &m_mainObj->m_materialConstsCPU.useMetallicMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap", &m_mainObj->m_materialConstsCPU.useRoughnessMap,
            1);

        if (flag) {
            m_mainObj->UpdateConstantBuffers(m_device, m_context);
        }

        ImGui::Checkbox("Draw Normals", &m_mainObj->m_drawNormals);

        ImGui::TreePop();
    }
}

} // namespace hlab