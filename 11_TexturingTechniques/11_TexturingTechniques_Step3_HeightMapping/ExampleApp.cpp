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

    // m_tesselatedQuad.Initialize(m_device);
    // m_tesselatedQuad.m_diffuseResView = m_cubeMapping.m_diffuseResView;
    // m_tesselatedQuad.m_specularResView = m_cubeMapping.m_specularResView;

    // 배경 나무 텍스춰
    vector<Vector4> points;
    Vector4 p = {-40.0f, 1.0f, 20.0f, 1.0f};
    for (int i = 0; i < 100; i++) {
        points.push_back(p);
        p.x += 2.0f;
    }
    std::vector<std::string> treeTextureFilenames = {
        "../Assets/Textures/TreeBillboards/1.png",
        "../Assets/Textures/TreeBillboards/2.png",
        "../Assets/Textures/TreeBillboards/3.png",
        "../Assets/Textures/TreeBillboards/4.png",
        "../Assets/Textures/TreeBillboards/5.png"};
    m_billboardPoints.Initialize(m_device, m_context, points, 2.4f,
                                 L"BillboardPointsPixelShader.hlsl",
                                 treeTextureFilenames);

    /* Fireball 초기화
    // Shadertoy Media Files
    https://shadertoyunofficial.wordpress.com/2019/07/23/shadertoy-media-files/
    m_fireballs.Initialize(m_device,
                           {{-0.7f, 0.5f, 1.2f, 1.0f},
                            {0.0f, 0.5f, 1.3f, 1.0f},

                             n{.7f, 0.5f, 1.2f, 1.0f}},
                           0.7f, L"FireballPixelShader.hlsl",
                           {"../Assets/Textures/shadertoy_fireball.jpg"});
    */

    m_cubeMapping.Initialize(
        m_device, L"../Assets/Textures/Cubemaps/skybox/cubemap_bgra.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_diffuse.dds",
        L"../Assets/Textures/Cubemaps/skybox/cubemap_specular.dds");

    // Main Sphere
    {
        Vector3 center(0.0f, 0.5f, 1.0f);
        float radius = 0.4f;
        MeshData sphere =
            GeometryGenerator::MakeSphere(radius, 100, 100, {1.0f, 1.0f});

        // 여러가지 텍스춰들!
        sphere.albedoTextureFilename =
            "../Assets/Textures/PBR/cgaxis_grey_porous_rock_40_56_4K/"
            "grey_porous_rock_40_56_diffuse.jpg";

        sphere.normalTextureFilename =
            "../Assets/Textures/PBR/cgaxis_grey_porous_rock_40_56_4K/"
            "grey_porous_rock_40_56_normal.jpg";

        sphere.heightTextureFilename =
            "../Assets/Textures/PBR/cgaxis_grey_porous_rock_40_56_4K/"
            "grey_porous_rock_40_56_height.jpg";
        m_mainSphere.m_basicPixelConstantData.reverseNormalMapY = true;

        sphere.aoTextureFilename =
            "../Assets/Textures/PBR/cgaxis_grey_porous_rock_40_56_4K/"
            "grey_porous_rock_40_56_ao.jpg";

        m_mainSphere.Initialize(m_device, m_context, {sphere});
        m_mainSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_mainSphere.m_specularResView = m_cubeMapping.m_specularResView;
        m_mainSphere.UpdateModelWorld(Matrix::CreateTranslation(center));
        m_mainSphere.m_basicPixelConstantData.useTexture = true;

        m_mainSphere.m_basicPixelConstantData.material.diffuse = Vector3(1.0f);
        m_mainSphere.m_basicPixelConstantData.material.specular = Vector3(0.0f);
        m_mainSphere.m_basicPixelConstantData.indexColor =
            Vector4(1.0f, 0.0, 0.0, 0.0);
        m_mainSphere.UpdateConstantBuffers(m_device, m_context);

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, radius);
    }

    // Cursor Sphere
    // Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.02f, 10, 10);
        m_cursorSphere.Initialize(m_device, m_context, {sphere});
        m_cursorSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_cursorSphere.m_specularResView = m_cubeMapping.m_specularResView;
        Matrix modelMat = Matrix::CreateTranslation({0.0f, 0.0f, 0.0f});
        Matrix invTransposeRow = modelMat;
        invTransposeRow.Translation(Vector3(0.0f));
        invTransposeRow = invTransposeRow.Invert().Transpose();
        m_cursorSphere.m_basicVertexConstantData.modelWorld =
            modelMat.Transpose();
        m_cursorSphere.m_basicVertexConstantData.invTranspose =
            invTransposeRow.Transpose();
        m_cursorSphere.m_basicPixelConstantData.useTexture = 0;
        m_cursorSphere.m_basicPixelConstantData.useNormalMap = 0;
        m_cursorSphere.m_basicPixelConstantData.useAOMap = 0;
        m_cursorSphere.m_basicVertexConstantData.useHeightMap = 0;
        m_cursorSphere.m_basicPixelConstantData.material.diffuse =
            Vector3(1.0f, 1.0f, 0.0f);
        m_cursorSphere.m_basicPixelConstantData.material.specular =
            Vector3(0.0f);
        m_cursorSphere.m_basicPixelConstantData.indexColor = Vector4(0.0f);
        m_cursorSphere.UpdateConstantBuffers(m_device, m_context);
    }


    // Ground
    {
        //MeshData ground = GeometryGenerator::MakeSquare(20.0f, {40.0f, 40.0f});
        MeshData ground = GeometryGenerator::MakeSquareGrid(2048, 2048, 20.0f,
                                                            {40.0f, 40.0f});
        ground.albedoTextureFilename =
            "../Assets/Textures/PBR/Rock053_1K-JPG/Rock053_1K_Color.jpg";

        ground.normalTextureFilename =
            "../Assets/Textures/PBR/Rock053_1K-JPG/"
            "Rock053_1K_NormalDX.jpg";

        ground.heightTextureFilename =
            "../Assets/Textures/PBR/Rock053_1K-JPG/"
            "Rock053_1K_Displacement.jpg";

        ground.aoTextureFilename = "../Assets/Textures/PBR/Rock053_1K-JPG/"
                                   "Rock053_1K_AmbientOcclusion.jpg";

        m_meshGroupGround.Initialize(m_device, m_context, {ground});
        m_meshGroupGround.m_diffuseResView = m_cubeMapping.m_diffuseResView;
        m_meshGroupGround.m_specularResView = m_cubeMapping.m_specularResView;
        m_meshGroupGround.UpdateModelWorld(
            Matrix::CreateRotationX(DirectX::XM_PIDIV2));
        m_meshGroupGround.m_basicPixelConstantData.useTexture = true;
        m_meshGroupGround.m_basicVertexConstantData.useHeightMap = false;
        m_meshGroupGround.m_basicPixelConstantData.useAOMap = false;
        m_meshGroupGround.m_basicPixelConstantData.material.diffuse =
            Vector3(1.0f);
        m_meshGroupGround.m_basicPixelConstantData.material.specular =
            Vector3(0.0f);
        m_meshGroupGround.UpdateConstantBuffers(m_device, m_context);
    }

    BuildFilters();

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
    m_cubeMapping.UpdateConstantBuffers(
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
            m_cursorSphere.m_basicVertexConstantData.view = viewRow.Transpose();
            m_cursorSphere.m_basicVertexConstantData.projection =
                projRow.Transpose();
            m_cursorSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
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
            m_cursorSphere.m_basicVertexConstantData.view = viewRow.Transpose();
            m_cursorSphere.m_basicVertexConstantData.projection =
                projRow.Transpose();
            m_cursorSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
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

    // m_tesselatedQuad.m_constantData.eyeWorld = eyeWorld;
    // m_tesselatedQuad.m_constantData.model = Matrix();
    // m_tesselatedQuad.m_constantData.view = viewRow.Transpose();
    // m_tesselatedQuad.m_constantData.proj = projRow.Transpose();
    // D3D11Utils::UpdateBuffer(m_device, m_context,
    //                          m_tesselatedQuad.m_constantData,
    //                          m_tesselatedQuad.m_constantBuffer);

    // 포인트 라이트 효과
    Light pointLight;
    pointLight.position = m_lightPosition;
    pointLight.strength = Vector3(2.0f);
    pointLight.fallOffEnd = 20.0f;

    m_meshGroupGround.m_basicPixelConstantData.lights[1] = pointLight;
    m_mainSphere.m_basicPixelConstantData.lights[1] = pointLight;
    m_cursorSphere.m_basicPixelConstantData.lights[1] = pointLight;

    m_meshGroupGround.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_meshGroupGround.m_basicVertexConstantData.view = viewRow.Transpose();
    m_meshGroupGround.m_basicVertexConstantData.projection =
        projRow.Transpose();
    m_meshGroupGround.UpdateConstantBuffers(m_device, m_context);

    m_billboardPoints.m_constantData.eyeWorld = eyeWorld;
    m_billboardPoints.m_constantData.view = viewRow.Transpose();
    m_billboardPoints.m_constantData.proj = projRow.Transpose();
    D3D11Utils::UpdateBuffer(m_device, m_context,
                             m_billboardPoints.m_constantData,
                             m_billboardPoints.m_constantBuffer);

    // m_fireballs.m_constantData.eyeWorld = eyeWorld;
    // m_fireballs.m_constantData.view = viewRow.Transpose();
    // m_fireballs.m_constantData.proj = projRow.Transpose();
    // m_fireballs.m_constantData.time += dt;
    // D3D11Utils::UpdateBuffer(m_device, m_context, m_fireballs.m_constantData,
    //                          m_fireballs.m_constantBuffer);

    // 물체 이동
    // 원점의 위치를 옮기지 않기 위해 Translation 추출
    Vector3 translation = m_mainSphere.m_modelWorldRow.Translation();
    // 회전만 남긴다.
    m_mainSphere.m_modelWorldRow.Translation(Vector3(0.0f));
    m_mainSphere.UpdateModelWorld(
        m_mainSphere.m_modelWorldRow * Matrix::CreateFromQuaternion(q) *
        Matrix::CreateTranslation(dragTranslation + translation));

    // Bounding Sphere도 같이 이동
    m_mainBoundingSphere.Center = m_mainSphere.m_modelWorldRow.Translation();
    m_mainSphere.m_basicVertexConstantData.view = viewRow.Transpose();
    m_mainSphere.m_basicVertexConstantData.projection = projRow.Transpose();
    m_mainSphere.m_basicPixelConstantData.eyeWorld = eyeWorld;
    m_mainSphere.UpdateConstantBuffers(m_device, m_context);

    if (m_dirtyflag && m_filters.size() > 1) {
        m_filters[1]->m_pixelConstData.threshold = m_threshold;
        m_filters[1]->UpdateConstantBuffers(m_device, m_context);
        m_filters.back()->m_pixelConstData.strength = m_strength;
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

    // 마우스 피킹에 사용할 indexRenderTarget도 초기화
    m_context->ClearRenderTargetView(m_indexRenderTargetView.Get(), clearColor);

    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    // Multiple render targets
    // 인덱스를 저장할 RenderTarget을 추가
    ID3D11RenderTargetView *targets[] = {m_renderTargetView.Get(),
                                         m_indexRenderTargetView.Get()};
    m_context->OMSetRenderTargets(2, targets, m_depthStencilView.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    if (m_drawAsWire) {
        m_context->RSSetState(m_wireRasterizerSate.Get());
    } else {
        m_context->RSSetState(m_rasterizerSate.Get());
    }

    // m_billboardPoints.Render(m_context);
    // m_fireballs.Render(m_context);
    m_mainSphere.Render(m_context);

    // m_tesselatedQuad.Render(m_context);

    if ((m_leftButton || m_rightButton) && m_selected)
        m_cursorSphere.Render(m_context);

    m_meshGroupGround.Render(m_context);

    // 물체 렌더링 후 큐브매핑
    // m_cubeMapping.Render(m_context);

    // 후처리 필터 시작하기 전에 Texture2DMS에 렌더링 된 결과를 Texture2D로 복사
    // MSAA Texture2DMS to Texture2D
    // https://stackoverflow.com/questions/24269813/directx-newb-multisampled-texture2d-with-depth-on-a-billboard
    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    m_context->ResolveSubresource(m_tempTexture.Get(), 0, backBuffer.Get(), 0,
                                  DXGI_FORMAT_R8G8B8A8_UNORM);

    // 후처리 필터
    if (m_usePostProcessing) {
        for (auto &f : m_filters) {
            f->Render(m_context);
        }
    }

    this->UpdateMousePickColor();
}

void ExampleApp::BuildFilters() {

    m_filters.clear();

    // 해상도를 낮춰서 다운 샘플링
    auto copyFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
                                 m_screenWidth, m_screenHeight);
    copyFilter->SetShaderResources({m_shaderResourceView});
    m_filters.push_back(copyFilter);

    for (int down = 2; down <= m_down; down *= 2) {
        auto downFilter = make_shared<ImageFilter>(
            m_device, m_context, L"Sampling", L"Sampling", m_screenWidth / down,
            m_screenHeight / down);

        if (down == 2) {
            downFilter->SetShaderResources({m_shaderResourceView});
            downFilter->m_pixelConstData.threshold = m_threshold;
        } else {
            downFilter->SetShaderResources(
                {m_filters.back()->m_shaderResourceView});
            downFilter->m_pixelConstData.threshold = 0.0f;
        }

        downFilter->UpdateConstantBuffers(m_device, m_context);
        m_filters.push_back(downFilter);
    }

    for (int down = m_down; down >= 1; down /= 2) {
        for (int i = 0; i < m_repeat; i++) {
            auto &prevResource = m_filters.back()->m_shaderResourceView;
            m_filters.push_back(make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"BlurX",
                m_screenWidth / down, m_screenHeight / down));
            m_filters.back()->SetShaderResources({prevResource});

            auto &prevResource2 = m_filters.back()->m_shaderResourceView;
            m_filters.push_back(make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"BlurY",
                m_screenWidth / m_down, m_screenHeight / m_down));
            m_filters.back()->SetShaderResources({prevResource2});
        }

        if (down > 1) {
            auto upFilter = make_shared<ImageFilter>(
                m_device, m_context, L"Sampling", L"Sampling",
                m_screenWidth / down * 2, m_screenHeight / down * 2);
            upFilter->SetShaderResources(
                {m_filters.back()->m_shaderResourceView});
            upFilter->m_pixelConstData.threshold = 0.0f;
            upFilter->UpdateConstantBuffers(m_device, m_context);
            m_filters.push_back(upFilter);
        }
    }

    auto combineFilter =
        make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Combine",
                                 m_screenWidth, m_screenHeight);
    combineFilter->SetShaderResources({copyFilter->m_shaderResourceView,
                                       m_filters.back()->m_shaderResourceView});
    combineFilter->SetRenderTargets(
        {this->m_renderTargetView}); // 렌더타겟 교체
    combineFilter->m_pixelConstData.strength = m_strength;
    combineFilter->UpdateConstantBuffers(m_device, m_context);
    m_filters.push_back(combineFilter);
}

void ExampleApp::UpdateGUI() {

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {

        ImGui::Checkbox("Use FPV", &m_useFirstPersonView);
        ImGui::Checkbox("Use PostProc", &m_usePostProcessing);
        if (ImGui::Checkbox("Draw Normals", &m_mainSphere.m_drawNormals)) {
            m_mainSphere.m_drawNormalsDirtyFlag = true;
        }
        ImGui::Checkbox("Wireframe", &m_drawAsWire);

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light")) {

        ImGui::SliderFloat3("Position", &m_lightPosition.x, -5.0f, 5.0f);

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sphere")) {

        bool temp = bool(m_mainSphere.m_basicPixelConstantData.useTexture);
        if (ImGui::Checkbox("Use AlbedoTexture", &temp)) {
            m_mainSphere.m_basicPixelConstantData.useTexture = int(temp);
        }

        temp = bool(m_mainSphere.m_basicPixelConstantData.useNormalMap);
        if (ImGui::Checkbox("Use NormalMapping", &temp)) {
            m_mainSphere.m_basicPixelConstantData.useNormalMap = int(temp);
        }

        temp = bool(m_mainSphere.m_basicPixelConstantData.useAOMap);
        if (ImGui::Checkbox("Use AO", &temp)) {
            m_mainSphere.m_basicPixelConstantData.useAOMap = int(temp);
        }

        temp = bool(m_mainSphere.m_basicVertexConstantData.useHeightMap);
        if (ImGui::Checkbox("Use HeightMapping", &temp)) {
            m_mainSphere.m_basicVertexConstantData.useHeightMap = int(temp);
        }

        ImGui::SliderFloat("HeightScale",
                           &m_mainSphere.m_basicVertexConstantData.heightScale,
                           0.0f, 0.1f);

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Ground")) {

        bool temp = bool(m_meshGroupGround.m_basicPixelConstantData.useTexture);

        if (ImGui::Checkbox("Use AlbedoTexture2", &temp)) {
            m_meshGroupGround.m_basicPixelConstantData.useTexture = int(temp);
        }

        temp = bool(m_meshGroupGround.m_basicPixelConstantData.useNormalMap);
        if (ImGui::Checkbox("Use NormalMapping2", &temp)) {
            m_meshGroupGround.m_basicPixelConstantData.useNormalMap = int(temp);
        }

        temp = bool(m_meshGroupGround.m_basicPixelConstantData.useAOMap);
        if (ImGui::Checkbox("Use AO2", &temp)) {
            m_meshGroupGround.m_basicPixelConstantData.useAOMap = int(temp);
        }

        temp = bool(m_meshGroupGround.m_basicVertexConstantData.useHeightMap);
        if (ImGui::Checkbox("Use HeightMapping2", &temp)) {
            m_meshGroupGround.m_basicVertexConstantData.useHeightMap =
                int(temp);
        }

        ImGui::SliderFloat(
            "HeightScale2",
            &m_meshGroupGround.m_basicVertexConstantData.heightScale, 0.0f,
            0.1f);

        ImGui::TreePop();
    }

    // int flag = 0;
    // flag += ImGui::SliderFloat4("Edges",
    //                             &m_tesselatedQuad.m_constantData.edges.x, 1,
    //                             8);
    // flag += ImGui::SliderFloat2(
    //     "Inside", &m_tesselatedQuad.m_constantData.inside.x, 1, 8);
    // if (flag) {
    //     D3D11Utils::UpdateBuffer(m_device, m_context,
    //                              m_tesselatedQuad.m_constantData,
    //                              m_tesselatedQuad.m_constantBuffer);
    // }

    m_dirtyflag = 0;
    // m_dirtyflag +=
    //     ImGui::SliderFloat("Bloom Threshold", &m_threshold, 0.0f, 1.0f);
    // m_dirtyflag +=
    //     ImGui::SliderFloat("Bloom Strength", &m_strength, 0.0f, 3.0f);
}

} // namespace hlab
