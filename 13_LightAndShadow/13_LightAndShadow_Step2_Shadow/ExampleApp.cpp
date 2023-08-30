#include "ExampleApp.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
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
        m_device, L"../../Assets/Textures/Cubemaps/HDRI/DayCamp/SampleEnvHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DayCamp/SampleSpecularHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DayCamp/SampleDiffuseHDR.dds",
        L"../../Assets/Textures/Cubemaps/HDRI/DayCamp/SampleBrdf.dds");

    // 조명 설정
    {
        // 위치와 방향은 Update()에서 설정
        m_light.radiance = Vector3(7.0f);
        m_light.spotPower = 6.0f;
        m_light.fallOffEnd = 20.0f;
        m_light.type = 0x02 | 0x10; // Spot with shadow
    }

    // 별도의 거울 물체를 만들지 않고 바닥을 거울로 구현
    /* {
        auto mesh = GeometryGenerator::MakeSquare(0.48f);
        m_mirror =
            make_shared<BasicMeshGroup>(m_device, m_context, vector{mesh});
        m_mirror->m_basicPixelConstData.material.albedo = Vector3(0.3f);
        m_mirror->m_basicPixelConstData.material.emission = Vector3(0.0f);
        m_mirror->m_basicPixelConstData.material.metallic = 0.7f;
        m_mirror->m_basicPixelConstData.material.roughness = 0.2f;

        m_mirror->UpdateModelWorld(
            Matrix::CreateScale(1.0f, 1.5f, 1.0f) *
            Matrix::CreateRotationY(3.141592f * 0.5f) *
            Matrix::CreateTranslation(0.5f, 0.25f, 2.0f));

        m_mirrorPlane = SimpleMath::Plane(Vector3(0.5f, 0.25f, 2.0f),
                                          Vector3(-1.0f, 0.0f, 0.0f));
    }*/

    // 바닥
    {
        auto mesh = GeometryGenerator::MakeSquare(5.0);
        mesh.albedoTextureFilename =
            "../../Assets/Textures/blender_uv_grid_2k.png";

        m_ground =
            make_shared<BasicMeshGroup>(m_device, m_context, vector{mesh});
        m_ground->m_basicPixelConstData.material.albedo = Vector3(0.5f);
        m_ground->m_basicPixelConstData.material.emission = Vector3(0.0f);
        m_ground->m_basicPixelConstData.material.metallic = 0.2f;
        m_ground->m_basicPixelConstData.material.roughness = 0.8f;

        m_ground->m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_ground->m_specularSRV = m_cubeMapping.m_specularSRV;
        m_ground->m_brdfSRV = m_cubeMapping.m_brdfSRV;

        m_ground->m_shadowSRV =
            this->m_shadowBuffer.m_resolvedSRV; // <- 그림자맵 추가
        m_ground->m_lightConstBuffer = this->m_lightEyeViewProjConstBuffer;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        m_ground->UpdateModelWorld(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                   Matrix::CreateTranslation(position));

        m_mirror = m_ground; // 바닥을 거울처럼 반사 구현
        // m_basicList.push_back(m_ground); // 리스트에 등록 X

        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
    }

    // Main Object
    {
        // auto meshes = GeometryGenerator::ReadFromFile(
        //     "../Assets/Models/DamagedHelmet/", "DamagedHelmet.gltf");

         auto meshes = GeometryGenerator::ReadFromFile(
             "../../Models/ToyCar/glTF/", "ToyCar.gltf",
             true);

        // 컴퓨터가 느릴 때는 간단한 물체로 테스트 하세요.
        //vector<MeshData> meshes = {GeometryGenerator::MakeSphere(0.4f, 50, 50)};

        // string path = "../Assets/Characters/armored-female-future-soldier/";
        // auto meshes = GeometryGenerator::ReadFromFile(path,
        // "angel_armor.fbx"); meshes[0].albedoTextureFilename = path +
        // "/angel_armor_albedo.jpg"; meshes[0].emissiveTextureFilename = path +
        // "/angel_armor_e.jpg"; meshes[0].metallicTextureFilename = path +
        // "/angel_armor_metalness.jpg"; meshes[0].normalTextureFilename = path
        // + "/angel_armor_normal.jpg"; meshes[0].roughnessTextureFilename =
        //     path + "/angel_armor_roughness.jpg";

        Vector3 center(0.0f, 0.0f, 2.0f);
        m_mainObj = make_shared<BasicMeshGroup>(m_device, m_context, meshes);
        m_mainObj->m_basicPixelConstData.invertNormalMapY =
            true; // GLTF는 true로
        m_mainObj->m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_mainObj->m_specularSRV = m_cubeMapping.m_specularSRV;
        m_mainObj->m_brdfSRV = m_cubeMapping.m_brdfSRV;
        m_mainObj->UpdateModelWorld(Matrix::CreateTranslation(center));
        m_mainObj->UpdateConstantBuffers(m_device, m_context);
        m_mainObj->m_basicPixelConstData.material.albedo = Vector3(1.0f);
        m_mainObj->m_basicPixelConstData.material.roughness = 1.0f;
        m_mainObj->m_basicPixelConstData.material.metallic = 1.0f;

        m_mainObj->m_shadowSRV =
            this->m_shadowBuffer.m_resolvedSRV; // <- 그림자맵
        m_mainObj->m_lightConstBuffer = this->m_lightEyeViewProjConstBuffer;
        m_mainObj->UpdateConstantBuffers(m_device, m_context);

        m_basicList.push_back(m_mainObj); // 리스트에 등록

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, 0.4f);
    }

    // 추가 물체들
    {
        MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
        Vector3 center(0.5f, 0.5f, 2.0f);
        auto m_obj =
            make_shared<BasicMeshGroup>(m_device, m_context, vector{mesh});
        m_obj->UpdateModelWorld(Matrix::CreateTranslation(center));
        m_obj->m_basicPixelConstData.material.albedo =
            Vector3(0.1f, 0.1f, 1.0f);
        m_obj->m_basicPixelConstData.material.roughness = 0.2f;
        m_obj->m_basicPixelConstData.material.metallic = 0.6f;
        m_obj->m_basicPixelConstData.material.emission = Vector3(0.0f);
        m_obj->m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_obj->m_specularSRV = m_cubeMapping.m_specularSRV;
        m_obj->m_brdfSRV = m_cubeMapping.m_brdfSRV;
        m_obj->UpdateConstantBuffers(m_device, m_context);

        m_obj->m_shadowSRV = this->m_shadowBuffer.m_resolvedSRV; // <- 그림자맵
        m_obj->m_lightConstBuffer = this->m_lightEyeViewProjConstBuffer;

        m_basicList.push_back(m_obj);
    }

    {
        MeshData mesh = GeometryGenerator::MakeBox(0.2f);
        Vector3 center(0.0f, 0.5f, 2.5f);
        auto m_obj =
            make_shared<BasicMeshGroup>(m_device, m_context, vector{mesh});
        m_obj->UpdateModelWorld(Matrix::CreateTranslation(center));
        m_obj->m_basicPixelConstData.material.albedo =
            Vector3(1.0f, 0.2f, 0.2f);
        m_obj->m_basicPixelConstData.material.roughness = 0.5f;
        m_obj->m_basicPixelConstData.material.metallic = 0.9f;
        m_obj->m_basicPixelConstData.material.emission = Vector3(0.0f);
        m_obj->m_irradianceSRV = m_cubeMapping.m_irradianceSRV;
        m_obj->m_specularSRV = m_cubeMapping.m_specularSRV;
        m_obj->m_brdfSRV = m_cubeMapping.m_brdfSRV;
        m_obj->UpdateConstantBuffers(m_device, m_context);

        m_obj->m_shadowSRV = this->m_shadowBuffer.m_resolvedSRV; // <- 그림자맵
        m_obj->m_lightConstBuffer = this->m_lightEyeViewProjConstBuffer;

        m_basicList.push_back(m_obj);
    }

    // 조명 위치 표시
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        m_lightSphere =
            make_shared<BasicMeshGroup>(m_device, m_context, vector{sphere});
        m_lightSphere->UpdateModelWorld(
            Matrix::CreateTranslation(m_light.position));
        m_lightSphere->m_basicPixelConstData.material.albedo = Vector3(0.0f);
        m_lightSphere->m_basicPixelConstData.material.emission =
            Vector3(1.0f, 1.0f, 0.0f);
        m_lightSphere->UpdateConstantBuffers(m_device, m_context);

        // m_basicList.push_back(m_lightSphere); // 그림자 X
    }

    // 커서 표시 (Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구)
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        m_cursorSphere.Initialize(m_device, m_context, vector{sphere});
        m_cursorSphere.UpdateModelWorld(
            Matrix::CreateTranslation(Vector3(0.0f)));
        m_cursorSphere.m_basicPixelConstData.material.albedo = Vector3(0.0f);
        m_cursorSphere.m_basicPixelConstData.material.emission =
            Vector3(0.0f, 1.0f, 0.0f);
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

    // 반사 행렬 추가
    Vector3 eyeWorld = m_camera.GetEyePos();
    Matrix reflectionRow = Matrix::CreateReflection(m_mirrorPlane);
    Matrix viewRow = m_camera.GetViewRow();
    Matrix projRow = m_camera.GetProjRow();

    AppBase::UpdateEyeViewProjBuffers(eyeWorld, viewRow, projRow,
                                      reflectionRow);

    // 조명 업데이트
    static Vector3 lightDev = Vector3(0.8f, 0.0f, 0.0f);
    if (m_lightRotate) { // 조명 회전
        lightDev = Vector3::Transform(
            lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
    }
    m_light.position = Vector3(0.0f, 1.5f, 2.0f) + lightDev;
    Vector3 focusPosition = Vector3(0.0f, 0.0f, 1.7f);
    m_light.direction = focusPosition - m_light.position;
    m_light.direction.Normalize();
    Matrix lightViewRow = XMMatrixLookAtLH(m_light.position, focusPosition,
                                           Vector3(0.0f, 1.0f, 0.0f));

    // 그림자 물체 업데이트
    m_lightSphere->UpdateModelWorld(
        Matrix::CreateTranslation(m_light.position));
    m_lightSphere->UpdateConstantBuffers(m_device, m_context);

    // 그림자맵 만들때 사용
    // Aspect Ratio = 1 사용 (나중에 샘플링할때 편함)
    // FOV는 스팟라이트 범위에 맞춰서 설정
    // 힌트: XMMatrixLookAtLH() 사용
    // 힌트: XMMatrixPerspectiveFovLH() 사용
    // TODO: m_lightEyeViewProjConstData.eyeWorld = ... ;
    // TODO: m_lightEyeViewProjConstData.viewProj = ... ;
    m_lightEyeViewProjConstData.depthPass = 1; // Depth만 출력할때 사용
    D3D11Utils::UpdateBuffer(m_device, m_context, m_lightEyeViewProjConstData,
                             m_lightEyeViewProjConstBuffer);

    // 큐브 매핑 Constant Buffer 업데이트
    m_cubeMapping.UpdateViewProjConstBuffer(m_device, m_context, viewRow,
                                            projRow, reflectionRow);

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

    // 거울은 따로 처리
    m_mirror->m_basicPixelConstData.lights[2] = m_light; // Spot Light
    m_mirror->UpdateConstantBuffers(m_device, m_context);

    // 마우스 이동/회전 반영
    Vector3 translation = m_mainObj->m_modelWorldRow.Translation();
    m_mainObj->m_modelWorldRow.Translation(Vector3(0.0f));
    m_mainObj->UpdateModelWorld(
        m_mainObj->m_modelWorldRow * Matrix::CreateFromQuaternion(q) *
        Matrix::CreateTranslation(dragTranslation + translation));
    m_mainBoundingSphere.Center = m_mainObj->m_modelWorldRow.Translation();

    for (auto &i : m_basicList) {
        // 조명 설정 (쉐이더의 Light 배열에서 인덱스 2이 Spot Light)
        i->m_basicPixelConstData.lights[2] = m_light;
        i->UpdateConstantBuffers(m_device, m_context);
    }
}

void ExampleApp::MainPass() {

    SetViewport();

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    vector<ID3D11RenderTargetView *> renderTargetViews = {
        m_mainBuffer.m_bufferRTV.Get()};

    // 디버깅할때 MSAA끄고 백버퍼에 직접 렌더
    // postProcessing.render()도 주석처리
    // vector<ID3D11RenderTargetView *> renderTargetViews = {
    //    m_backBufferRTV.Get()};

    for (size_t i = 0; i < renderTargetViews.size(); i++) {
        m_context->ClearRenderTargetView(renderTargetViews[i], clearColor);
    }
    m_context->OMSetRenderTargets(UINT(renderTargetViews.size()),
                                  renderTargetViews.data(),
                                  m_depthStencilView.Get());

    /* 거울 1. 원래 대로 한 번 그림 */

    // 기본 BlendState 사용
    m_context->OMSetBlendState(NULL, NULL, 0xffffffff);

    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);

    m_context->OMSetDepthStencilState(m_drawDSS.Get(), 1);

    m_context->RSSetState(m_drawAsWire ? m_wireRS.Get() : m_solidRS.Get());

    for (auto &i : m_basicList) {
        i->Render(m_context, AppBase::m_eyeViewProjConstBuffer, m_useEnv);
    }

    // 거울은 빼고 그립니다.
    // m_mirror->Render(m_context, AppBase::m_eyeViewProjConstBuffer, m_useEnv);

    if ((m_leftButton || m_rightButton) && m_selected)
        m_cursorSphere.Render(m_context, AppBase::m_eyeViewProjConstBuffer,
                              false);

    if (m_useEnv) {
        m_cubeMapping.Render(m_context, false);
    }

    m_lightSphere->Render(m_context, AppBase::m_eyeViewProjConstBuffer,
                          m_useEnv);

    /* 거울 2. 거울 위치만 StencilBuffer에 1로 표기 */

    // STENCIL만 클리어
    // 거울을 가리는 물체가 있을 수도 있어서 Depth는 CLEAR 안함
    // 앞 단계의 m_drawDSS에서 모두 KEEP을 사용했기 때문에
    // Stencil도 CLEAR 불필요
    // m_context->ClearDepthStencilView(m_depthStencilView.Get(),
    //                                 D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 두 번째 UINT StencilRef = 1 사용
    // ClearDepthStencilView(..., 0)에서는 다른 숫자 0 사용
    m_context->OMSetDepthStencilState(m_maskDSS.Get(), 1);

    // 거울을 그릴 때 색은 필요 없기 때문에 간단한 PS 사용 가능
    m_mirror->Render(m_context, AppBase::m_eyeViewProjConstBuffer, m_useEnv);

    /* 거울 3. 거울 위치에 반사된 물체들을 렌더링 */
    // 주의
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH, 1.0f, 0);
    m_context->OMSetDepthStencilState(m_drawMaskedDSS.Get(), 1);

    // 반사되면 삼각형 정점들의 순서(Winding)가 반대로 -> 반시계
    m_context->RSSetState(m_drawAsWire ? m_wireCCWRS.Get()
                                       : m_solidCCWRS.Get());

    // 반사된 위치에 그려야 함
    for (auto &i : m_basicList) {
        i->Render(m_context, AppBase::m_mirrorEyeViewProjConstBuffer, m_useEnv);
    }

    if (m_useEnv) {
        m_cubeMapping.Render(m_context, true);
    }

    /* 거울 4. 거울 자체의 재질을 "Blend"로 그림 */

    const float t = m_mirrorAlpha;
    const float blendColor[] = {t, t, t, 1.0f};
    m_context->OMSetBlendState(m_mirrorBS.Get(), blendColor, 0xffffffff);

    m_context->RSSetState(m_drawAsWire ? m_wireRS.Get() : m_solidRS.Get());
    m_mirror->Render(m_context, AppBase::m_eyeViewProjConstBuffer, m_useEnv);

    // 후처리는 Blend X
    m_context->OMSetBlendState(NULL, NULL, 0xffffffff);

    /* 이후 원래 하던 후처리 */

    m_mainBuffer.Resolve(m_context);
}

void ExampleApp::DepthPass(TextureBuffer &textureBuffer,
                           ComPtr<ID3D11Buffer> &depthEyeViewProjConstBuffer) {

    // depthEyeViewProjConstBuffer 사용

    // SetViewport();
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = float(textureBuffer.m_width);
    shadowViewport.Height = float(textureBuffer.m_height);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &shadowViewport);

    const float clearColor[4] = {100.0f, 0.0f, 0.0f, 1.0f};
    vector<ID3D11RenderTargetView *> renderTargetViews = {
        textureBuffer.m_bufferRTV.Get()};

    // vector<ID3D11RenderTargetView *> renderTargetViews = {
    //     m_backBufferRTV.Get()};

    for (size_t i = 0; i < renderTargetViews.size(); i++) {
        m_context->ClearRenderTargetView(renderTargetViews[i], clearColor);
    }

    if (m_useShadow) {

        m_context->OMSetRenderTargets(UINT(renderTargetViews.size()),
                                      renderTargetViews.data(),
                                      m_shadowDSV.Get());

        // 기본 BlendState 사용
        m_context->OMSetBlendState(NULL, NULL, 0xffffffff);

        m_context->ClearDepthStencilView(
            m_shadowDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,
            0);

        m_context->OMSetDepthStencilState(m_drawDSS.Get(), 1);

        m_context->RSSetState(m_drawAsWire ? m_wireRS.Get() : m_solidRS.Get());

        for (auto &i : m_basicList) {
            i->Render(m_context, depthEyeViewProjConstBuffer, m_useEnv);
        }

        m_mirror->Render(m_context, depthEyeViewProjConstBuffer, m_useEnv);
    }

    // 환경큐브 생략

    textureBuffer.Resolve(m_context);
}

void ExampleApp::Render() {

    // 카메라 시점에서 깊이값 렌더링 (MainPass 주석처리할 것)
    DepthPass(m_mainBuffer, m_depthEyeViewProjConstBuffer);
    
    // 조명 시점에서 깊이값 렌더링 (MainPass 주석처리할 것)
    //DepthPass(m_mainBuffer, m_lightEyeViewProjConstBuffer);
    
    // 조명 시점의 깊이값을 그림자맵으로 렌더링
    //DepthPass(m_shadowBuffer, m_lightEyeViewProjConstBuffer);

    //MainPass(); // 기본 렌더링 (반사 포함)

    // 디버깅하기 위해서 backbuffer 에 직접 렌더링할 때는 주석 처리
    m_postProcess.Render(m_context);
}

void ExampleApp::UpdateGUI() {

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
            CheckMSAA();
            CreateBuffers();
        }
        ImGui::Checkbox("UseShadow", &m_useShadow);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Env Map")) {
        ImGui::Checkbox("Use Env", &m_useEnv);
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

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
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
        int flag = 0;
        flag += ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Metallic", &m_mirror->m_basicPixelConstData.material.metallic,
            0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &m_mirror->m_basicPixelConstData.material.roughness,
            0.0f, 1.0f);
        if (flag) {
            m_mirror->UpdateConstantBuffers(m_device, m_context);
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Point Light")) {
        ImGui::SliderFloat3("Position", &m_light.position.x, -5.0f, 5.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material")) {

        int flag = 0;

        flag += ImGui::SliderFloat(
            "Metallic", &m_ground->m_basicPixelConstData.material.metallic,
            0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Roughness", &m_ground->m_basicPixelConstData.material.roughness,
            0.0f, 1.0f);
        flag += ImGui::CheckboxFlags(
            "AlbedoTexture", &m_mainObj->m_basicPixelConstData.useAlbedoMap, 1);
        flag += ImGui::CheckboxFlags(
            "EmissiveTexture", &m_mainObj->m_basicPixelConstData.useEmissiveMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use NormalMapping", &m_mainObj->m_basicPixelConstData.useNormalMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use AO", &m_mainObj->m_basicPixelConstData.useAOMap, 1);
        flag += ImGui::CheckboxFlags(
            "Use HeightMapping",
            &m_mainObj->m_basicVertexConstData.useHeightMap, 1);
        flag += ImGui::SliderFloat(
            "HeightScale", &m_mainObj->m_basicVertexConstData.heightScale, 0.0f,
            0.1f);
        flag += ImGui::CheckboxFlags(
            "Use MetallicMap", &m_mainObj->m_basicPixelConstData.useMetallicMap,
            1);
        flag += ImGui::CheckboxFlags(
            "Use RoughnessMap",
            &m_mainObj->m_basicPixelConstData.useRoughnessMap, 1);
        flag += ImGui::Checkbox("Draw Normals", &m_mainObj->m_drawNormals);

        if (flag) {
            // GUI 입력이 있을 때만 할 일들 추가
        }

        ImGui::TreePop();
    }
}
} // namespace hlab
