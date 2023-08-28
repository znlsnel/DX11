#include "ExampleApp.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::Initialize() {

    if (!AppBase::Initialize())
        return false;

    m_cubeMapping.Initialize(
        m_device, L"../../Assets/Textures/Cubemaps/HDRI/DaySky/dayskyEnvHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DaySky/dayskySpecularHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DaySky/dayskyDiffuseHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DaySky/dayskyBrdf.dds");

    // m_cubeMapping.Initialize(
    //    m_device, L"../Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
    //    L"../Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
    //    L"../Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
    //    L"../Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");

    // Main Sphere
    {
        Vector3 center(0.0f, 0.0f, 1.0f);
        float radius = 0.4f;
        MeshData sphere =
            GeometryGenerator::MakeSphere(radius, 100, 100, {2.0f, 2.0f});

        // PBR 텍스춰들
        sphere.albedoTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_albedo.png";
        sphere.normalTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_normal-dx.png";
        sphere.heightTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_height.png";
        sphere.aoTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_ao.png";
        sphere.metallicTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_metallic.png";
        sphere.roughnessTextureFilename =
            "../../Assets/Textures/PBR/worn-painted-metal-ue/"
            "worn-painted-metal_roughness.png";

        m_mainSphere.Initialize(m_device, m_context, {sphere});
        m_mainSphere.m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_mainSphere.m_specularSRV = m_cubeMapping.m_specularSRV;
        m_mainSphere.m_brdfSRV = m_cubeMapping.m_brdfSRV;
        m_mainSphere.UpdateModelWorld(Matrix::CreateTranslation(center));
        m_mainSphere.UpdateConstantBuffers(m_device, m_context);

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, radius);
    }

    // Cursor Sphere
    // Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.02f, 10, 10);
        m_cursorSphere.Initialize(m_device, m_context, {sphere});
        m_cursorSphere.m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_cursorSphere.m_specularSRV = m_cubeMapping.m_specularSRV;
        Matrix modelMat = Matrix::CreateTranslation({0.0f, 0.0f, 0.0f});
        Matrix invTransposeRow = modelMat;
        invTransposeRow.Translation(Vector3(0.0f));
        invTransposeRow = invTransposeRow.Invert().Transpose();
        m_cursorSphere.m_basicVertexConstData.modelWorld = modelMat.Transpose();
        m_cursorSphere.m_basicVertexConstData.invTranspose =
            invTransposeRow.Transpose();
        m_cursorSphere.UpdateConstantBuffers(m_device, m_context);
    }

    return true;
}

void ExampleApp::Update(float dt) {

    // 카메라의 이동
    if (m_useFirstPersonView) {
        if (m_keyPressed[87]) // W키
            m_camera.MoveForward(dt);
        if (m_keyPressed[83]) // S키
            m_camera.MoveForward(-dt);
        if (m_keyPressed[68]) // D키
            m_camera.MoveRight(dt);
        if (m_keyPressed[65]) // A키
            m_camera.MoveRight(-dt);
        if (m_keyPressed[81]) // Q키
            m_camera.MoveUp(dt);
        if (m_keyPressed[69]) // E키
            m_camera.MoveUp(-dt);
    }

    Matrix viewRow = m_camera.GetViewRow();
    Matrix projRow = m_camera.GetProjRow();
    Vector3 eyeWorld = m_camera.GetEyePos();

    // 큐브 매핑 Constant Buffer 업데이트
    Matrix viewEnvRow = viewRow;
    viewEnvRow.Translation(Vector3(0.0f)); // 이동 취소
    m_cubeMapping.UpdateVertexConstBuffer(
        m_device, m_context, viewEnvRow.Transpose(), projRow.Transpose());

    // mainSphere의 회전 계산용
    static float prevRatio = 0.0f;
    static Vector3 prevPos(0.0f);
    static Vector3 prevVector(0.0f);
    Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 dragTranslation(0.0f);

    // 마우스 왼쪽 버튼으로 회전
    if (m_leftButton) {

        // ViewFrustum에서 가까운 면 위의 커서 위치
        // ViewFrustum에서 먼 면 위의 커서 위치
        Vector3 cursorNdcNear = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_cursorNdcX, m_cursorNdcY, 1.0f);

        // NDC 커서 위치를 월드 좌표계로 역변환 해주는 행렬
        Matrix inverseProjView = (viewRow * projRow).Invert();

        // ViewFrustum 안에서 PickingRay의 방향 구하기
        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, inverseProjView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, inverseProjView);
        Vector3 dir = cursorWorldFar - cursorWorldNear;
        dir.Normalize();

        // 광선을 만들고 충돌 감지
        SimpleMath::Ray curRay = SimpleMath::Ray(cursorWorldNear, dir);
        float dist = 0.0f;
        m_selected = curRay.Intersects(m_mainBoundingSphere, dist);

        if (m_selected) {

            Vector3 pickPoint = cursorWorldNear + dist * dir;

            // 충돌 지점에 작은 구 그리기
            m_cursorSphere.UpdateModelWorld(
                Matrix::CreateTranslation(pickPoint));
            m_cursorSphere.m_basicVertexConstData.view = viewRow.Transpose();
            m_cursorSphere.m_basicVertexConstData.projection =
                projRow.Transpose();
            m_cursorSphere.m_basicPixelConstData.eyeWorld = eyeWorld;
            m_cursorSphere.UpdateConstantBuffers(m_device, m_context);

            // mainSphere를 어떻게 회전시킬지 결정
            if (m_dragStartFlag) { // 드래그를 시작하는 경우
                m_dragStartFlag = false;
                prevVector = pickPoint - m_mainBoundingSphere.Center;
                prevVector.Normalize();
            } else {
                Vector3 currentVector = pickPoint - m_mainBoundingSphere.Center;
                currentVector.Normalize();
                float theta = acos(prevVector.Dot(currentVector));
                if (theta > 3.141592f / 180.0f * 3.0f) {
                    Vector3 axis = prevVector.Cross(currentVector);
                    axis.Normalize();
                    q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                                                                    theta);
                    prevVector = currentVector;
                }

                // 마우스가 조금이라도 움직였을 경우에만 회전시키기
                // if ((currentVector - prevVector).Length() > 1e-3) {
                //    // q = SimpleMath::Quaternion::FromToRotation(prevVector,
                //    // currentVector);
                //    // prevVector = currentVector;
                //}
            }
        }
    }

    // 마우스 오른쪽 버튼으로 이동
    if (m_rightButton) {

        // ViewFrustum에서 가까운 면 위의 커서 위치
        // ViewFrustum에서 먼 면 위의 커서 위치
        Vector3 cursorNdcNear = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_cursorNdcX, m_cursorNdcY, 1.0f);

        // NDC 커서 위치를 월드 좌표계로 역변환 해주는 행렬
        Matrix inverseProjView = (viewRow * projRow).Invert();

        // ViewFrustum 안에서 PickingRay의 방향 구하기
        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, inverseProjView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, inverseProjView);
        Vector3 dir = cursorWorldFar - cursorWorldNear;
        dir.Normalize();

        // 광선을 만들고 충돌 감지
        SimpleMath::Ray curRay = SimpleMath::Ray(cursorWorldNear, dir);
        float dist = 0.0f;
        m_selected = curRay.Intersects(m_mainBoundingSphere, dist);

        if (m_selected) {

            Vector3 pickPoint = cursorWorldNear + dist * dir;

            // 충돌 지점에 작은 구 그리기
            m_cursorSphere.UpdateModelWorld(
                Matrix::CreateTranslation(pickPoint));
            m_cursorSphere.m_basicVertexConstData.view = viewRow.Transpose();
            m_cursorSphere.m_basicVertexConstData.projection =
                projRow.Transpose();
            m_cursorSphere.m_basicPixelConstData.eyeWorld = eyeWorld;
            m_cursorSphere.UpdateConstantBuffers(m_device, m_context);

            // mainSphere를 어떻게 회전시킬지 결정
            if (m_dragStartFlag) { // 드래그를 시작하는 경우
                m_dragStartFlag = false;
                prevRatio = dist / (cursorWorldFar - cursorWorldNear).Length();
                prevPos = pickPoint;
            } else {
                Vector3 newPos = cursorWorldNear +
                                 prevRatio * (cursorWorldFar - cursorWorldNear);

                // 마우스가 조금이라도 움직였을 경우에만 회전시키기
                if ((newPos - prevPos).Length() > 1e-3) {
                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
            }
        }
    }

    // 포인트 라이트 효과
    Light pointLight;
    pointLight.position = m_lightPosition;
    pointLight.radiance = Vector3(1.0f); // Strength
    pointLight.fallOffEnd = 20.0f;

    m_mainSphere.m_basicPixelConstData.lights[1] = pointLight;
    m_cursorSphere.m_basicPixelConstData.lights[1] = pointLight;

    Vector3 translation = m_mainSphere.m_modelWorldRow.Translation();
    m_mainSphere.m_modelWorldRow.Translation(Vector3(0.0f));
    m_mainSphere.UpdateModelWorld(
        m_mainSphere.m_modelWorldRow * Matrix::CreateFromQuaternion(q) *
        Matrix::CreateTranslation(dragTranslation + translation));

    m_mainBoundingSphere.Center = m_mainSphere.m_modelWorldRow.Translation();
    m_mainSphere.m_basicVertexConstData.view = viewRow.Transpose();
    m_mainSphere.m_basicVertexConstData.projection = projRow.Transpose();
    m_mainSphere.m_basicPixelConstData.eyeWorld = eyeWorld;
    m_mainSphere.UpdateConstantBuffers(m_device, m_context);
}

void ExampleApp::Render() {

    SetViewport();

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    vector<ID3D11RenderTargetView *> renderTargetViews = {m_floatRTV.Get()};
    for (size_t i = 0; i < renderTargetViews.size(); i++) {
        m_context->ClearRenderTargetView(renderTargetViews[i], clearColor);
    }
    m_context->OMSetRenderTargets(UINT(renderTargetViews.size()),
                                  renderTargetViews.data(),
                                  m_depthStencilView.Get());

    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);

    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    if (m_drawAsWire) {
        m_context->RSSetState(m_wireRasterizerSate.Get());
    } else {
        m_context->RSSetState(m_solidRasterizerSate.Get());
    }

    m_mainSphere.Render(m_context);

    if ((m_leftButton || m_rightButton) && m_selected)
        m_cursorSphere.Render(m_context);

    // 물체 렌더링 후 큐브매핑
    m_cubeMapping.Render(m_context);

    m_context->ResolveSubresource(m_resolvedBuffer.Get(), 0,
                                  m_floatBuffer.Get(), 0,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);

    m_postProcess.Render(m_context);
}

void ExampleApp::UpdateGUI() {

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
            CreateBuffers();
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Env Map")) {
        int flag = 0;
        flag += ImGui::RadioButton(
            "Env", &m_cubeMapping.m_pixelConstData.textureToDraw, 0);
        ImGui::SameLine();
        flag += ImGui::RadioButton(
            "Specular", &m_cubeMapping.m_pixelConstData.textureToDraw, 1);
        ImGui::SameLine();
        flag += ImGui::RadioButton(
            "Irradiance", &m_cubeMapping.m_pixelConstData.textureToDraw, 2);
        flag += ImGui::SliderFloat(
            "Mip Level", &m_cubeMapping.m_pixelConstData.mipLevel, 0.0f, 10.0f);
        if (flag) {
            m_cubeMapping.UpdatePixelConstBuffer(m_device, m_context);
        }
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
    if (ImGui::TreeNode("Point Light")) {
        ImGui::SliderFloat3("Position", &m_lightPosition.x, -5.0f, 5.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material")) {

        int flag = 0;

        flag += ImGui::SliderFloat(
            "Metallic", &m_mainSphere.m_basicPixelConstData.material.metallic,
            0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &m_mainSphere.m_basicPixelConstData.material.roughness,
            0.0f, 1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &m_mainSphere.m_basicPixelConstData.useAlbedoMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping",
            &m_mainSphere.m_basicPixelConstData.useNormalMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &m_mainSphere.m_basicPixelConstData.useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping",
            &m_mainSphere.m_basicVertexConstData.useHeightMap, 1);
        flag += ImGui::SliderFloat(
            "HeightScale", &m_mainSphere.m_basicVertexConstData.heightScale,
            0.0f, 0.1f);
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap",
            &m_mainSphere.m_basicPixelConstData.useMetallicMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &m_mainSphere.m_basicPixelConstData.useRoughnessMap, 1);
        flag += ImGui::Checkbox("Draw Normals", &m_mainSphere.m_drawNormals);

        if (flag) {
            // GUI 입력이 있을 때만 할 일들 추가
        }

        ImGui::TreePop();
    }
}
} // namespace hlab
