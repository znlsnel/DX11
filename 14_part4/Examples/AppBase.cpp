#include "queue"

#include "Model.h"
#include "AppBase.h"
#include "Character.h"
#include "D3D11Utils.h"
#include "JsonManager.h"
#include "InputManager.h"
#include "GraphicsCommon.h"
#include "TessellationModel.h"

#include <algorithm>
#include <filesystem>
#include <directxtk/SimpleMath.h>



// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

AppBase *g_appBase = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_mainWindow(0),
      m_screenViewport(D3D11_VIEWPORT()) 
{

    g_appBase = this;
     
            m_camera = make_shared<Camera>(g_appBase);
    m_inputManager = make_shared<InputManager>(this);
    // m_JsonManager->TestJson_Parse();
    // m_JsonManager->TestJson_AddMember();
    m_camera->SetAspectRatio(this->GetAspectRatio());

}

AppBase::~AppBase() {
   // m_JsonManager->SaveMesh();
    g_appBase = nullptr;

    m_JsonManager->SaveMesh();
    m_objects.clear();
    
    // Cleanup
    ImGui_ImplDX11_Shutdown(); 
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_mainWindow); 
}
  
float AppBase::GetAspectRatio() const {

        float ratio = float(m_screenWidth) / m_screenHeight;

        //std::cout << "ratio : " << ratio << std::endl;
    return ratio;

}
bool AppBase::MouseObjectPicking() {

    if ( m_keyPressed['Q'] || m_keyPressed['W']) {
        return false;
    }



    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));

    m_context->ResolveSubresource(m_indexTempTexture.Get(), 0,
                                  m_indexTexture.Get(), 0,
                                  DXGI_FORMAT_R8G8B8A8_UNORM);

    //D3D11Utils::WriteToPngFile(m_device, m_context, m_indexTempTexture,
      //                         "captured.png");

    return ReadPixelOfMousePos<UINT8>(m_device, m_context);
}

Vector3 AppBase::RayCasting(bool editTexture, float mouseNdcX, float mouseNdcY) { 
       mouseNdcX = mouseNdcX == -10.f ? m_mouseNdcX : mouseNdcX;
        mouseNdcY = mouseNdcY == -10.f ? m_mouseNdcY : mouseNdcY;


       //if (MouseObjectPicking() == false || m_pickedModel == nullptr)
       //         return;
       // 

        Matrix viewRow = m_camera->GetViewRow();
        Matrix projRow = m_camera->GetProjRow();
        Vector3 eyeWorld = m_camera->GetEyePos();
         

        Vector3 cursorNdcNear = Vector3(mouseNdcX, mouseNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(mouseNdcX, mouseNdcY, 1.0f);
        Matrix inverseProjView = (viewRow * projRow).Invert();

        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, inverseProjView);

        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, inverseProjView);

        Vector3 dir = cursorWorldFar - cursorWorldNear;
        dir.Normalize();
        Vector3 tempPos;
       float fDist = FLT_MAX;
        int fLevel = 0;
          
         
        for (auto &BVH : m_groundPlane->m_BVHs) {
                std::queue<pair<int, int>> queue;
                queue.push(make_pair(0,0));

                while (queue.empty() == false) {
                    int level = queue.front().second; 
                int index = queue.front().first;
                queue.pop();

                SimpleMath::Ray ray =
                        SimpleMath::Ray(cursorWorldNear, dir);
                float dist = 0.0f;

                Vector3 tempCenter = BVH[index].m_bb.Center;
                Vector3 tempExtents = BVH[index].m_bb.Extents;

                Matrix temp = m_groundPlane->m_worldRow;

                BVH[index].m_bb.Center =
                        Vector3::Transform(tempCenter, temp);
                temp.Translation(Vector3(0.0f));
                BVH[index].m_bb.Extents =
                        Vector3::Transform(tempExtents, temp);


                bool m_selected =
                        BVH[index].Intersects(cursorWorldNear, dir, dist);
                        
                // bool m_selected = ray.Intersects(BVH[index], dist);

                        if (m_selected) {
                                if (fLevel < level || (fLevel == level && fDist > dist)) {
                                fDist = dist;
                                fLevel = level;
                                }

                                int left = (index * 2) + 1;
                                int right = left + 1;

                                bool hasLeftChild = left < BVH.size();
                                bool hasRightChild = right < BVH.size();
                                 
                                if (hasLeftChild)
                                        queue.push(make_pair(left, level+1));
                                if (hasRightChild)
                                        queue.push(make_pair(right, level + 1));
                        }
                        BVH[index].m_bb.Center = tempCenter;
                        BVH[index].m_bb.Extents = tempExtents;
                }
        }
        
       if (fDist < FLT_MAX) { 
                       Vector3 pos = cursorWorldNear + dir * fDist;
                if (editTexture)
                        m_groundPlane->UpdateTextureMap(m_context, pos,
                                                        m_textureType);  
                return pos;

       } 
       return Vector3(0.0f, 0.0f, 0.0f); 
}

void AppBase::RayCasting(Vector3 origin, Vector3 dir, float& dist) {

       float fDist = FLT_MAX;
       int fLevel = 0;
       int resultIndex = -1;
       bool hasLeftChild = false;
       bool hasRightChild = false;
       Matrix tempRow = m_groundPlane->m_worldRow;
       for (auto &BVH : m_groundPlane->m_BVHs) {

                std::queue<pair<int, int>> queue;
                queue.push(make_pair(0, 0));

                while (queue.empty() == false) {
                        int level = queue.front().second;
                        int index = queue.front().first;
                        queue.pop();

                        float dist = 0.0f;

                        Vector3 tempCenter = BVH[index].m_bb.Center;
                        Vector3 tempExtents = BVH[index].m_bb.Extents;

                        
                        bool m_selected = false;

                        BVH[index].m_bb.Center =
                                Vector3::Transform(tempCenter, tempRow);
                        tempRow.Translation(Vector3(0.0f));
                        BVH[index].m_bb.Extents =
                                Vector3::Transform(tempExtents, tempRow);

                        m_selected = BVH[index].Intersects(origin, dir, dist);      
                        
                        if (m_selected) 
                        {
                                if (fLevel < level ||
                                        (fLevel == level && fDist > dist)) {
                                        fDist = dist;
                                        fLevel = level;
                                        resultIndex = index;
                                }

                                int left = (index * 2) + 1;
                                int right = left + 1;

                                hasLeftChild = left < BVH.size();
                                hasRightChild = right < BVH.size();

                                if (hasLeftChild)
                                        queue.push(make_pair(left, level + 1));
                                if (hasRightChild)
                                        queue.push(make_pair(right, level + 1));
                        }
                        BVH[index].m_bb.Center = tempCenter;
                        BVH[index].m_bb.Extents = tempExtents;
                 }
       }

       if (fDist < FLT_MAX) {
                 m_groundPlane->m_BVHs[0][resultIndex].updateWorldVertex(
                     tempRow);
                 bool r = m_groundPlane->m_BVHs[0][resultIndex].TriangleIntersects(origin, dir, dist);
                       
                 if (r == false) 
                         dist = fDist;

                 if (r) {
                         cout << "succeeed \n";
                 }
                 else {
                         cout << "failed  \n";
                 }

                cout << "vertexSize : "
                      << int(m_groundPlane->m_BVHs[0][resultIndex]
                            .worldVertexs.size())
                 << "\n";

                for (auto &s : m_cursorSphere)
                         s->m_isVisible = false;

                int size =
                    m_groundPlane->m_BVHs[0][resultIndex].worldVertexs.size();
                for (int i = 0; i < size; i++) {
                         if (m_cursorSphere.size() <= i)
                                break;

                        m_cursorSphere[i]->m_isVisible = true;
                         m_cursorSphere[i]->UpdatePosition(
                             m_groundPlane->m_BVHs[0][resultIndex]
                                 .worldVertexs[i]);
                 }
       } 
}
 
void AppBase::SetHeightPosition(Vector3 origin, Vector3 dir, float &dist) { 
        float dv = 1024.f / 500.f * 0.5f;
        float a = 1024.f / 60.f;
        float rv = 60.f / 500.f * 0.5f;
           
        // up down left right pos; 
        origin -= m_groundPlane->GetPosition(); 
        int currPos[2] = {int(a * (origin.x + 30.0f)),
                          int(a * (origin.z + 30.0f))
        }; 

        float upPos[2] = {round((a * (origin.x + 30.0f) + dv)/ (dv * 2)) * (dv * 2),
            round((a * (origin.z + 30.0f) + dv) / (dv * 2)) * (dv * 2)
        }; 
        
        float downPos[2] = {round((a * (origin.x + 30.0f) - dv) / (dv * 2)) *
                                (dv * 2),
            round((a * (origin.z + 30.0f) - dv) / (dv * 2)) * (dv * 2)
        }; 

        float leftPos[2] = {round((a * (origin.x + 30.0f) - dv) / (dv * 2)) *
                                (dv * 2),
            round((a * (origin.z + 30.0f) + dv) / (dv * 2)) * (dv * 2)
        }; 
        float rightPos[2] = {round((a * (origin.x + 30.0f) + dv) / (dv * 2)) *
                                 (dv * 2),
            round((a * (origin.z + 30.0f) - dv) / (dv * 2)) * (dv * 2)
        };  

         auto changePos = [&](float idxPos[2], Vector3& pos) { 
                 idxPos[0] = clamp(idxPos[0], 0.0f, 1024.f);
                 idxPos[1] = clamp(idxPos[1], 0.0f, 1024.f);

                 int idx = int(idxPos[1]) * 1024 * 4 + int(idxPos[0]) * 4;
                 idx = clamp(idx, 0, int(heightMapImage.size() - 1));
                 float h = heightMapImage[idx] / 255.f * 8.0f;
                  
                 pos.x = roundf(pos.x / (rv * 2)) * (rv * 2);
                 pos.z = roundf(pos.z / (rv * 2)) * (rv * 2);
                 pos.y = h;
        };

         Vector3 upPosition = Vector3(origin.x + rv, 0.0f, origin.z + rv);
         Vector3 downPosition = Vector3(origin.x - rv, 0.0f, origin.z - rv);
         Vector3 leftPosition = Vector3(origin.x - rv, 0.0f, origin.z + rv);
         Vector3 rightPosition = Vector3(origin.x + rv, 0.0f, origin.z - rv);
          
         changePos(upPos, upPosition);
         changePos(downPos, downPosition);
         changePos(leftPos, leftPosition);
         changePos(rightPos, rightPosition); 

         //m_cursorSphere[1]->UpdatePosition(upPosition);
         //m_cursorSphere[2]->UpdatePosition(downPosition);
         //m_cursorSphere[3]->UpdatePosition(leftPosition);
         //m_cursorSphere[4]->UpdatePosition(rightPosition); 

         DirectX::SimpleMath::Ray ray = SimpleMath::Ray(origin, dir);
         bool result = ray.Intersects(downPosition, leftPosition, upPosition, dist);
         if (result == false)
                 result = 
                     ray.Intersects(downPosition, upPosition, rightPosition,
                                         dist);
}

void AppBase::ObjectDrag() {
    if (m_pickedModel == nullptr || m_leftButton == false)
        return;

        Matrix viewRow = m_camera->GetViewRow();
        Matrix projRow = m_camera->GetProjRow();
        Vector3 eyeWorld = m_camera->GetEyePos();

        static float prevRatio = 0.0f;
        static Vector3 prevPos(0.0f);
        static Vector3 prevVector(0.0f);

        Vector3 dragTranslation(0.0f);

        Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);

        if (m_keyPressed['Q']) {
            //    cout << "Object Drag" << endl;

                Vector3 cursorNdcNear = Vector3(m_mouseNdcX, m_mouseNdcY, 0.0f);
                Vector3 cursorNdcFar = Vector3(m_mouseNdcX, m_mouseNdcY, 1.0f);
                Matrix inverseProjView = (viewRow * projRow).Invert();

                Vector3 cursorWorldNear =
                        Vector3::Transform(cursorNdcNear, inverseProjView);
                Vector3 cursorWorldFar =
                        Vector3::Transform(cursorNdcFar, inverseProjView);

                Vector3 dir = cursorWorldFar - cursorWorldNear;
                dir.Normalize();

                SimpleMath::Ray ray = SimpleMath::Ray(cursorWorldNear, dir);
                float dist = 0.0f;
                m_selected = ray.Intersects(m_pickedModel->m_boundingSphere, dist);

                if (m_selected == false)
                    cout << "No Touch" << endl;
                if (m_selected) 
                {
                        Vector3 pickPoint = cursorWorldNear + dist * dir;
                    m_cursorSphere[0]->UpdatePosition(pickPoint);

                        if (m_dragStartFlag) 
                        {
                                m_dragStartFlag = false;
                                prevRatio = dist / (cursorWorldFar - cursorWorldNear).Length();
                                prevPos = pickPoint;
                        } 
                        else 
                        {
                                float currDist =
                                        prevRatio * (cursorWorldFar - cursorWorldNear).Length();
                                Vector3 currPickPoint = cursorWorldNear + dir * currDist;

                                if ((currPickPoint - prevPos).Length() > 1e-3) 
                                {
                                        dragTranslation = currPickPoint - prevPos;
                                        //dragTranslation.y = prevPos.y;
                                        prevPos = currPickPoint;
                                        Vector3 pos =
                                            m_pickedModel->GetPosition() +
                                            dragTranslation;
                                        m_pickedModel->UpdatePosition(pos);
                                }
                        }
                        
                } 
                

        } 
        else if (m_keyPressed['W'] && false) {

                Vector3 cursorNdcNear{m_mouseNdcX, m_mouseNdcY, 0.f};
                Vector3 cursorNdcFar{m_mouseNdcX, m_mouseNdcY, 1.f};

                Matrix inverseProjView = (viewRow * projRow).Invert();

                Vector3 cursorWorldNear =
                    Vector3::Transform(cursorNdcNear, inverseProjView);

                Vector3 cursorWorldFar =
                    Vector3::Transform(cursorNdcFar, inverseProjView);

                Vector3 dir = cursorWorldFar - cursorWorldNear;
                dir.Normalize();

                

                SimpleMath::Ray r = SimpleMath::Ray(cursorWorldNear, dir);
                float dist = 0.0f;
                m_selected =
                    r.Intersects(m_pickedModel->m_boundingSphere, dist);
                if (m_selected) 
                {
                        Vector3 pickPoint = cursorWorldNear + dist * dir;

                        if (m_dragStartFlag) 
                        {
                                m_dragStartFlag = false;
                                prevVector =
                                    pickPoint -
                                    m_pickedModel->m_boundingSphere.Center;
                                prevVector.Normalize();
                        } 
                        else 
                        {
                                Vector3 currentVector =
                                    pickPoint -
                                    m_pickedModel->m_boundingSphere.Center;
                                currentVector.Normalize();

                               //m_cursorSphere[1]->UpdatePosition(
                               //     m_pickedModel->m_boundingSphere.Center);
                               // m_cursorSphere[2]->UpdatePosition(
                               //     m_pickedModel->m_boundingSphere.Center +
                               //     currentVector);

                                float rotateTheta =
                                    acos(prevVector.Dot(currentVector));

                                if (rotateTheta > 3.141592f / 180.f) 
                                {
                                        Vector3 RotateAxis =
                                            prevVector.Cross(currentVector);
                                       RotateAxis.Normalize();
                                        
                                        Quaternion rotation =
                                           Quaternion::CreateFromAxisAngle(
                                               RotateAxis, rotateTheta);
                                         
                                        Vector3 currRot =
                                            m_pickedModel->GetRotation();

                                        Vector3 RotResult = Vector3::Transform(currRot, rotation);
                                    
                                        m_pickedModel->UpdateRotation(
                                            RotResult);

                                        prevVector = currentVector;
                                }
                        }
                }
        }
}

void AppBase::DestroyObject(shared_ptr<class Model> object) {

        object->DestroyObject();

    if (m_pickedModel == object)
        m_pickedModel = nullptr;
}
int AppBase::Run() {
         
    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Scene Control");
            // ImGui가 측정해주는 Framerate 출력 
            ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
             
            UpdateGUI(); // 추가적으로 사용할 GUI
             
            Update(ImGui::GetIO().DeltaTime);

            Render(); // <- 중요: 우리가 구현한 렌더링

            ImGui::End();
            ImGui::Render();



            
            // Example의 Render()에서 RT 설정을 해주지 않았을 경우에도
            // 백 버퍼에 GUI를 그리기위해 RT 설정
            // 예) Render()에서 ComputeShader만 사용
            AppBase::SetMainViewport();
            m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(),
                                          NULL);

            // GUI 렌더링
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // GUI 렌더링 후에 Present() 호출
            m_swapChain->Present(1, 0);
        }
    }

    return 0;
}

bool AppBase::Initialize() {
         
    if (!InitMainWindow())
        return false;  
     
    if (!InitDirect3D())    
        return false;

    if (!InitGUI())
        return false;

    if (!InitScene())
        return false;
     

    m_JsonManager = make_shared<JsonManager>(this);

    // PostEffect에 사용
    m_screenSquare = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeSquare()});

    // 환경 박스 초기화
    MeshData skyboxMesh = GeometryGenerator::MakeBox(500.0f);
    std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
    m_skybox = make_shared<Model>(m_device, m_context, vector{skyboxMesh});
    m_skybox->m_name = "SkyBox";
    m_skybox->m_useLod = false;
    // 콘솔창이 렌더링 창을 덮는 것을 방지
    SetForegroundWindow(m_mainWindow); 

        // Plane   
    if (true) { 
        string heightMapPath;
        bool hasHeightMap = false; 
         
        auto filePath = std::filesystem::current_path();
        for (const auto file : std::filesystem::directory_iterator(filePath)) {
            if (file.path().stem() == "heightMap") {
                                hasHeightMap = true;
                                heightMapPath = file.path().string();
                                break;
            }     
        }     
        if (hasHeightMap) { 
            D3D11Utils::ReadImageFile(heightMapPath, heightMapImage);
        }  
                
        Vector2 mapTexScale = Vector2(50.f, 50.f); 
        int mapArea = 100;  
        float mapScale = 30.f;
        auto mesh = GeometryGenerator::MakeTessellationPlane(
            mapArea, mapArea, mapScale, mapTexScale,
            heightMapImage);
        string path = 
            "../Assets/Textures/PBR/TerrainTextures/Ground037_4K-PNG/";
        mesh.albedoTextureFilename = path + "Ground037_4K-PNG_Color.png";
        mesh.aoTextureFilename = path + "Ground037_4K-PNG_AmbientOcclusion.png";
        mesh.normalTextureFilename = path + "Ground037_d4K-PNG_NormalDX.png";
        // mesh.roughnessTextureFilename = path +
        // "Ground037_4K-PNG_Roughness.png";
        mesh.heightTextureFilename = path + "Ground037_4K-PNG_Displacement.png";

        m_groundPlane = make_shared<TessellationModel>(
            m_device, m_context, vector{mesh}, this, true);
        m_groundPlane->mapScale = mapScale;
        m_groundPlane->texScale = mapTexScale;
        m_groundPlane->mapArea[0] = mapArea;
        m_groundPlane->mapArea[1] = mapArea;
         
        m_groundPlane->m_materialConsts.GetCpu().albedoFactor = Vector3(0.2f);
        m_groundPlane->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        m_groundPlane->m_materialConsts.GetCpu().metallicFactor = 0.f;
        m_groundPlane->m_materialConsts.GetCpu().roughnessFactor = 1.0f;
        m_groundPlane->m_materialConsts.GetCpu().useNormalMap = 1;
        m_groundPlane->m_materialConsts.GetCpu().useAOMap = 0;
        m_groundPlane->UpdateRotation(
            Vector3(90 * 3.141592f / 180.f, 0.0f, 0.0f));
        shared_ptr<Model> temp = m_groundPlane;
        temp->isObjectLock = true;
        AddBasicList(temp, false);
    }
  

    m_JsonManager->LoadMesh();
    return true;
}

// 여러 예제들이 공통적으로 사용하기 좋은 장면 설정
bool AppBase::InitScene() {
         
    // 조명 설정  
    {  
                for (int i = 0; i < MAX_LIGHTS; i++) {
                        m_globalConstsCPU.lights[i].radiance = Vector3(5.0f);
                        m_globalConstsCPU.lights[i].position =
                            Vector3(0.0f, 2.698f, -0.159f);
                         m_globalConstsCPU.lights[i].direction =
                            Vector3(0.85f, -0.781f, 0.625f);  
                        //m_globalConstsCPU.lights[i].direction =
                        //    Vector3(2.173f, -3.142f, 0.0f);  
                          // 2.173, -3.142, 0.0
                        m_globalConstsCPU.lights[i].direction.Normalize();
                        m_globalConstsCPU.lights[i].spotPower = 3.0f;
                        m_globalConstsCPU.lights[i].radius = 0.131f;
                        m_globalConstsCPU.lights[i].type =
                            LIGHT_DIRECTIONAL
                                | LIGHT_SHADOW; // Point with shadow
                }
                 
    } 

    // 조명 위치 표시
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            m_lightSphere[i] =
                make_shared<Model>(m_device, m_context, vector{sphere});
            m_lightSphere[i]->UpdatePosition(
                m_globalConstsCPU.lights[i].position);
            m_lightSphere[i]->m_materialConsts.GetCpu().albedoFactor =
                Vector3(0.0f);
            m_lightSphere[i]->m_materialConsts.GetCpu().emissionFactor =
                Vector3(1.0f, 1.0f, 0.0f);
            m_lightSphere[i]->m_castShadow =
                false; // 조명 표시 물체들은 그림자 X

            // if (m_globalConstsCPU.lights[i].type == 0)
            m_lightSphere[i]->m_isVisible = false;
            m_lightSphere[i]->m_name = "LightSphere" + std::to_string(i);
            m_lightSphere[i]->m_isPickable = false;

            AddBasicList(m_lightSphere[i], false);

        }
    }
     
    // 커서 표시 (Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구) 
    for (int i = 0; i < 4; i++)
    { 
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        shared_ptr<Model>cursorSphere =
            make_shared<Model>(m_device, m_context, vector{sphere});
        cursorSphere->m_isVisible = true; // 마우스가 눌렸을 때만 보임
        cursorSphere->m_castShadow = false; // 그림자 X
        cursorSphere->m_materialConsts.GetCpu().albedoFactor = Vector3(0.0f);
        cursorSphere->m_materialConsts.GetCpu().emissionFactor =
            Vector3(1.0f, 0.0f, 0.0f);
        
        cursorSphere->UpdateScale(Vector3(30.f, 30.f, 30.f));
        cursorSphere->isCursorShpere = true;
        m_cursorSphere.push_back(cursorSphere);
        
        AddBasicList(cursorSphere, false);
    }
    return true;
}

void AppBase::UpdateGUI() {
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImPos = Vector2(pos.x, pos.y);
    ImSize = Vector2(size.x, size.y);
   //     float tempWidth = ImGui::GetWindowSize().x;
   // float posX = ImGui::GetWindowPos().x;

   //     if (m_imGuiWidth == tempWidth && posX != 0.0f)
   //     return; 

   // ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
   // ImGui::SetWindowSize(ImVec2(m_imGuiWidth, m_screenHeight));
   // m_imGuiWidth = tempWidth;

   // ResizeSwapChain(m_screenWidth , m_screenHeight);
   //std::cout << "ImGui Width : " << m_imGuiWidth << std::endl;
    }
 
void AppBase::Update(float dt) {

        m_dt = dt;
        m_inputManager->Update(dt);
          
        for (auto sphere : m_cursorSphere) {
                float length =
            (sphere->GetPosition() - m_camera->GetPosition())
                        .Length() * 5;
                length = max(length, 5.0f);

                sphere->UpdateScale(Vector3(length) * 2);
        }
           
        static float frustumTimer = 0.0f;
        if (frustumTimer > 1.0f / 30.f) {
                GetObjectsInFrustum(false); 
                if (m_foundMirror)
                        GetObjectsInFrustum(true);
                frustumTimer = 0.0f;
        } 
        frustumTimer += dt;
         
        for (const auto &model : m_characters) {
                model->Update(dt);
        }


    // 카메라의 이동 
    //m_camera.UpdateKeyboard(dt, m_keyPressed);
        m_camera->UpdatePos();
    // 반사 행렬 추가
    const Vector3 eyeWorld = m_camera->GetEyePos();
        if (m_mirror) { 
                Matrix temp = m_mirror->m_worldRow;
                temp.Translation(Vector3(0.0f));
                      
                m_mirrorPlane =
                   SimpleMath::Plane(m_mirror->GetPosition(), 
                    Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), temp));
            
        } 
        m_reflectRow = Matrix::CreateReflection(m_mirrorPlane);
    const Matrix viewRow = m_camera->GetViewRow();
    const Matrix projRow = m_camera->GetProjRow();

    UpdateLights(dt); 

    // 공용 ConstantBuffer 업데이트
    AppBase::UpdateGlobalConstants(dt, eyeWorld, viewRow, projRow,
                                   m_reflectRow);
     
    // 거울은 따로 처리
    if (m_mirror) 
        m_mirror->UpdateConstantBuffers(m_device, m_context);

    // 조명의 위치 반영 
    for (int i = 0; i < MAX_LIGHTS; i++) {

        float scale = std::max(0.01f, m_globalConstsCPU.lights[i].radius);
        m_lightSphere[i]->UpdateTranseform(
            Vector3(scale), Vector3(0.0f),
            m_globalConstsCPU.lights[i].position);
    }

    // m_ground->UpdateConstantBuffers(m_device, m_context);
    //ProcessMouseControl();
    for (auto &i : m_basicList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }
    for (auto &i : m_NoneBVHList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }
     

} 

void AppBase::UpdateLights(float dt) {

    // 회전하는 lights[1] 업데이트
    static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
    if (m_lightRotate) {
        lightDev = Vector3::Transform(
            lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
    }    
       
    static float updateTimer = 0.0f;
    if (updateTimer > 0.0f) {

        Vector3 tempCamPos =
            m_camera->m_objectTargetCameraMode == false || m_camera->GetTarget() == nullptr
            ? RayCasting(false, 0.0f, -1.f)  
                :  m_camera->GetTarget()->GetMesh()->GetPosition();
          
        //Vector3 tempCamPos = m_camera->GetPosision();
         
        //Vector3 tempCamPos = RayCasting(0.0f, -0.25f);  
        float updateDistance = 10.f;
        tempCamPos.x = round(tempCamPos.x * updateDistance) / updateDistance;
        tempCamPos.y = round(tempCamPos.y * updateDistance) / updateDistance;
        tempCamPos.z = round(tempCamPos.z * updateDistance) / updateDistance;
         
         

        Vector3 tempLPos = m_globalConstsCPU.lights[0].direction;
        tempCamPos += -tempLPos * 5;

        for (int i = 0; i < 5; i++) 
                m_globalConstsCPU.lights[i].position = tempCamPos;

        updateTimer = 0.0f;  
    } 
    updateTimer += dt; 

    // 그림자맵을 만들기 위한 시점
    for (int i = 0; i < MAX_LIGHTS; i++) {
        auto &light = m_globalConstsCPU.lights[i];
        if (light.type & LIGHT_SHADOW) {

            UpdateLightInfo(m_shadowGlobalConstsGPU[i],
                            m_shadowGlobalConstsCPU[i], light,
                            m_shadowAspects[i]);
        }
    }
}  
  
void AppBase::UpdateLightInfo(
    ComPtr<ID3D11Buffer> &shadowGlobalConstsGPU, GlobalConstants &shadowGlobalConstants,
                              Light &light, Vector3 &aspect) {
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
        up = Vector3(1.0f, 0.0f, 0.0f); 
     
      
      
    // 그림자맵을 만들 때 필요
    Matrix lightViewRow =
        XMMatrixLookAtLH(light.position, light.position + light.direction, up);

    Matrix lightProjRow =
        m_camera->GetShadowProjRow(Vector2(aspect.x, aspect.y), aspect.z);
    shadowGlobalConstants.eyeWorld = light.position;
    shadowGlobalConstants.cameraWorld = m_camera->GetPosition();
    // m_shadowGlobalConstsCPU[i].eyeWorld =
    //     light.position +
    //     -light.direction * 3.f;

    shadowGlobalConstants.view = lightViewRow.Transpose();
    shadowGlobalConstants.proj = lightProjRow.Transpose();
    shadowGlobalConstants.invProj = lightProjRow.Invert().Transpose();
    shadowGlobalConstants.viewProj = (lightViewRow * lightProjRow).Transpose();

    // LIGHT_FRUSTUM_WIDTH 확인
    // Vector4 eye(0.0f, 0.0f, 0.0f, 1.0f);
    // Vector4 xLeft(-1.0f, -1.0f, 0.0f, 1.0f);
    // Vector4 xRight(1.0f, 1.0f, 0.0f, 1.0f);
    // eye = Vector4::Transform(eye, lightProjRow);
    // xLeft = Vector4::Transform(xLeft, lightProjRow.Invert());
    // xRight = Vector4::Transform(xRight, lightProjRow.Invert());
    // xLeft /= xLeft.w;
    // xRight /= xRight.w;
    // cout << "LIGHT_FRUSTUM_WIDTH = " << xRight.x - xLeft.x <<
    // endl;

    D3D11Utils::UpdateBuffer(m_context, shadowGlobalConstants,
                             shadowGlobalConstsGPU);

    // 그림자를 실제로 렌더링할 때 필요
    light.viewProj = shadowGlobalConstants.viewProj;
    light.invProj = shadowGlobalConstants.invProj;

    // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
    // 넣어주면 됩니다.
}

void AppBase::RenderDepthOnly(){

    // Clear   -       -       -       -       -       -       -       - 
    
    m_context->ClearDepthStencilView(m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH , 1.0f, 0); 
    
    // Set -    -       -       -       -       -       -       -       - 
    m_context->OMSetRenderTargets(0, m_indexRenderTargetView.GetAddressOf(),
                                  m_depthOnlyDSV.Get()); 
 
    AppBase::SetGlobalConsts(m_globalConstsGPU);

    for (const auto &model : m_foundModelList) {
        AppBase::SetPipelineState(model->GetDepthOnlyPSO());
        model->Render(m_context);
    }
    for (const auto &model : m_NoneBVHList) {
        AppBase::SetPipelineState(model->GetDepthOnlyPSO());
        model->Render(m_context);
    } 


    AppBase::SetPipelineState(Graphics::depthOnlyPSO);
    if (m_skybox)
        m_skybox->Render(m_context);
    if (m_mirror)
        m_mirror->Render(m_context);

}   
  
void AppBase::RenderShadowMaps() { 

    // 쉐도우 맵을 다른 쉐이더에서 SRV 해제
    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    m_context->PSSetShaderResources(15, 2, nulls);
     
    AppBase::SetShadowViewport(); // 그림자맵 해상도
       
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (m_globalConstsCPU.lights[i].type & LIGHT_SHADOW) {
            m_context->OMSetRenderTargets(0, NULL, 
                    m_shadowDSVs[i].Get());

            m_context->ClearDepthStencilView(m_shadowDSVs[i].Get(),
                                             D3D11_CLEAR_DEPTH, 1.0f, 0);

            AppBase::SetGlobalConsts(m_shadowGlobalConstsGPU[i]);

            
            for (const auto &model : m_basicList) {
                if (model->m_castShadow && model->m_isVisible) {
                    AppBase::SetPipelineState(model->GetDepthOnlyPSO());
                    model->Render(m_context);
                }
            }
            for (const auto &model : m_NoneBVHList) {
                if (model->m_castShadow && model->m_isVisible) {
                    AppBase::SetPipelineState(model->GetDepthOnlyPSO());
                    model->Render(m_context);
                }
            }
             
            if (m_mirror && m_mirror->m_castShadow)
                m_mirror->Render(m_context);
        } 
    }

} 

void AppBase::RenderOpaqueObjects() { 
    // 다시 렌더링 해상도로 되돌리기
    AppBase::SetMainViewport(); 

    // 거울 1. 거울은 빼고 원래 대로 그리기
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_floatRTV.Get(), clearColor);
    m_context->ClearRenderTargetView(m_indexRenderTargetView.Get(), clearColor); 
    ID3D11RenderTargetView *targets[] = {                     
        m_floatRTV.Get(), m_indexRenderTargetView.Get()
    };

    m_context->ClearDepthStencilView(
        m_defaultDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_context->OMSetRenderTargets(2, targets,
                                  m_defaultDSV.Get());   
        
    // 그림자맵들도 공용 텍스춰들 이후에 추가
    // 주의: 마지막 shadowDSV를 RenderTarget에서 해제한 후 설정
    vector<ID3D11ShaderResourceView *> shadowSRVs;
    for (int i = 0; i < MAX_LIGHTS; i++) { 
        shadowSRVs.push_back(m_shadowSRVs[i].Get()); 
    } 
     
    m_context->PSSetShaderResources(15, UINT(shadowSRVs.size()),
                                    shadowSRVs.data());
    
    

    AppBase::SetGlobalConsts(m_globalConstsGPU);

    // 스카이박스 그리기 
    // 투명한 물체가 있어서 편의상 다른 물체들보다 먼저 그렸습니다.
    // 최적화를 하고 싶다면 투명한 물체들만 따로 마지막에 그리면 됩니다.
    AppBase::SetPipelineState(m_drawAsWire ? Graphics::skyboxWirePSO
                                           : Graphics::skyboxSolidPSO);
    m_skybox->Render(m_context);
      
        for (const auto &model : m_foundModelList) {
                AppBase::SetPipelineState(model->GetPSO(m_drawAsWire));
                model->Render(m_context); 
        }
     
        for (const auto &model : m_NoneBVHList) {
                AppBase::SetPipelineState(model->GetPSO(m_drawAsWire));
                model->Render(m_context);
                 
        } 

    
    //AppBase::SetPipelineState(m_groundPlane->GetPSO(m_drawAsWire));
    //m_groundPlane->RenderTessellation(m_context);

    //if (m_selected)
    //    m_cursorSphere->Render(m_context);
    
    // AppBase::SetPipelineState(m_ground->GetTerrainPSO(m_drawAsWire));

    // 거울 반사를 그릴 필요가 없으면 불투명 거울만 그리기
    if (m_mirrorAlpha == 1.0f && m_mirror) {
        AppBase::SetPipelineState(m_drawAsWire ? Graphics::defaultWirePSO
                                               : Graphics::defaultSolidPSO);
        m_mirror->Render(m_context);
    }
    
    // 노멀 벡터 그리기
    AppBase::SetPipelineState(Graphics::normalsPSO);
    for (auto &model : m_basicList) {
        if (model->m_drawNormals) 
            model->RenderNormals(m_context);
    }




    AppBase::SetPipelineState(Graphics::boundingBoxPSO);
    if (AppBase::m_drawOBB) {
        for (auto &model : m_basicList) {
            model->RenderWireBoundingBox(m_context);
        }
    }
    if (m_bRenderingBVH)
        RenderBVH(); 


    if (m_pickedModel && m_pickedModel->bRenderingBVH) {
        m_pickedModel->RenderBVH(m_context);
    }

     

    //m_drawBS = m_keyPressed['W'] && m_camera->m_isCameraLock && m_camera->m_objectTargetCameraMode == false;

    if (AppBase::m_drawBS ) {
        for (auto &model : m_basicList) {
            model->RenderWireBoundingSphere(m_context);
        }
    }
}
//mirrorNormal : 0, 0, -1 
// reflectCameraPos : -0.530229, 4.93401, 4.53336 
// cameraPos : -6.96181,  4.93849,  -0.880365 
// mirrorPos : -6.43158, 0.00447464, 3.653
//        
   
void AppBase::RenderMirror() {
    if ( m_mirrorAlpha < 1.0f &&
        m_mirror) { // 거울 반사를 그려야 하는 상황 
         
        // 거울 2. 거울 위치만 StencilBuffer에 1로 표기
        AppBase::SetPipelineState(Graphics::stencilMaskPSO);
        m_mirror->Render(m_context);
          
        // 거울 3. 거울 위치에 반사된 물체들을 렌더링
        AppBase::SetGlobalConsts(m_reflectGlobalConstsGPU);
        m_context->ClearDepthStencilView(m_defaultDSV.Get(), D3D11_CLEAR_DEPTH,
                                         1.0f, 0); 
          
        for (auto &model : m_foundReflectModelList) {
            AppBase::SetPipelineState(model->GetReflectPSO(m_drawAsWire));
            model->Render(m_context);
        }
        static int tempTT = 0;
        if (tempTT > 5) {
                cout << "m_foundReflectModelList. size : " << int(m_foundReflectModelList.size()) << "\n";
            tempTT = 0;
        }
        tempTT++;

        for (auto &model : m_NoneBVHList) {
            AppBase::SetPipelineState(model->GetReflectPSO(m_drawAsWire));
            model->Render(m_context);
        }   
        
        AppBase::SetPipelineState(m_drawAsWire
                                      ? Graphics::reflectSkyboxWirePSO
                                     : Graphics::reflectSkyboxSolidPSO);
        m_skybox->Render(m_context); 
         
        // 거울 4. 거울 자체의 재질을 "Blend"로 그림
        AppBase::SetPipelineState(m_drawAsWire ? Graphics::mirrorBlendWirePSO
                                               : Graphics::mirrorBlendSolidPSO);
 
        AppBase::SetGlobalConsts(m_globalConstsGPU);
        m_mirror->Render(m_context);

    } // end of if (m_mirrorAlpha < 1.0f)
}

void AppBase::Render() {

    AppBase::SetMainViewport();

    // 공통으로 사용하는 샘플러들 설정
    m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->DSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->HSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                        Graphics::sampleStates.data());
    m_context->CSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());

    // 공통으로 사용할 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        m_envSRV.Get(), m_specularSRV.Get(), m_irradianceSRV.Get(),
        m_brdfSRV.Get()};
    m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());
      
    RenderDepthOnly();
     
    isRenderShadowMap = true;
    RenderShadowMaps(); 
    isRenderShadowMap = false;
     
    RenderOpaqueObjects();

    RenderMirror();
}

void AppBase::OnMouseMove(int mouseX, int mouseY) {


    m_mouseX = mouseX ;
    m_mouseY = mouseY;
    //cout << "mouse xy : " << m_mouseX << ", " << m_mouseY << endl;

    // 마우스 커서의 위치를 NDC로 변환
    // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
    // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
    m_mouseNdcX = m_mouseX * 2.0f / m_screenWidth - 1.0f;
    m_mouseNdcY = -m_mouseY * 2.0f / m_screenHeight + 1.0f;

    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
    //m_camera->RotateCamera(m_mouseNdcX, m_mouseNdcY);

    m_mouseNdcX = std::clamp(m_mouseNdcX, -1.0f, 1.0f);
    m_mouseNdcY = std::clamp(m_mouseNdcY, -0.7f, 0.7f);

    // 카메라 시점 회전
    ObjectDrag();
}
 
void AppBase::RenderBVH() {

    int startIndex = 0;
    int maxIndex = 0;
    for (int i = 0; i < maxBVHRenderLevel - 1; i++) {
        //startIndex = startIndex * 2 + 2;
        // if (i == maxRenderingBVHLevel - 3)
        //     startIndex = maxIndex;
        maxIndex = startIndex * 2 + 2;

    }  
    //maxIndex = startIndex * 2 + 2;
    startIndex++;
    //    cout << "maxIndex : " << maxIndex << endl;
    cout << "RenderBVH! !\n";

        for (int i = startIndex; i < m_BVNMeshs.size(); i++) {

                if (i > maxIndex)
                break;
                
                // cout << "rendering BVH level : " << i << "\n";
                ID3D11Buffer *constBuffers[2] = {
                    m_BVNMeshs[i]->meshConstsGPU.Get(),
                    m_BVNMeshs[i]->materialConstsGPU.Get()};
                
                m_context->VSSetConstantBuffers(1, 2, constBuffers);

                m_context->IASetVertexBuffers(
                    0, 1, m_BVNMeshs[i]->vertexBuffer.GetAddressOf(),
                    &m_BVNMeshs[i]->stride, &m_BVNMeshs[i]->offset);
                m_context->IASetIndexBuffer(m_BVNMeshs[i]->indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
                m_context->DrawIndexed(m_BVNMeshs[i]->indexCount, 0, 0);
        } 
    
}
   
void AppBase::OnMouseClick(int mouseX, int mouseY) 
{
    m_mouseX = mouseX;
    m_mouseY = mouseY; 
     
    m_mouseNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_mouseNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;
}
void AppBase::GetObjectsInFrustum(bool isMirror) {

        vector<shared_ptr<Model>> *modelList;
        if (isMirror) {
                modelList = &m_foundReflectModelList;
        } 
        else {
                modelList = &m_foundModelList;
                m_foundMirror = false;
        }
        modelList->resize(0);  
        
        MyFrustum frustum;
        frustum.InitFrustum(this, isMirror);
        
    std::queue<pair<BVNode, int>> queue;   
          
    if (m_BVNodes.size() == 0)
                UpdateBVH();
    if (m_BVNodes.size() == 0)
                return;
          
    static int id = 0; 
    queue.push(make_pair(m_BVNodes[0], 0)); 
      
    while (!queue.empty()) {
                BVNode &node = queue.front().first;
                int index = queue.front().second;
                queue.pop();
                  
                Vector3 center = node.boundingBox.Center;
                Vector3 Extents = node.boundingBox.Extents;  
                Matrix t; 
                 
                if (node.objectID >= 0) { 
                        center =
                            m_basicList[node.objectID]->m_boundingBox.Center;
                        Extents =
                            m_basicList[node.objectID]->m_boundingBox.Extents;
                        t = m_basicList[node.objectID]->m_worldRow;
                         
                        if ( isMirror) 
                                t = t * m_reflectRow; 
                } 
                else if (isMirror) {
                        center = Vector3::Transform(center, m_reflectRow);
                        Vector3 tempP = m_reflectRow.Translation();
                        m_reflectRow.Translation(Vector3());
                        Extents = Vector3::Transform(Extents, m_reflectRow);
                          
                        m_reflectRow.Translation(tempP);
                }
                 
                //center:
                //-0.000543907, 0, -0.000157923 
                //        ReflectCenter : -0.000543907, 0,  7.97932  
                //        center : -7.649,1.50192, 4.878 
                //        ReflectCenter : -7.649, 1.50192, 3.10116

                bool check = frustum.Intersects(center, Extents, t);
                   
                if (check) 
                {     
                        if (node.objectID >= 0) {
                                if (m_basicList[node.objectID] != m_mirror)
                                        modelList->push_back(m_basicList[node.objectID]);
                                else if (isMirror == false)
                                        m_foundMirror = true;
                                //{
                                //        cout << "center : " << prevCenter.x
                                //             << ", " << prevCenter.y << ", "
                                //             << prevCenter.z
                                //             << "\n";
                                //        cout << "ReflectCenter : " << center.x
                                //             << ", " << center.y << ", "
                                //             << center.z << "\n";
                                //}
                        }   
                          
                        int leftID = m_BVNodes[index].leftChildID;
                        int rightID = m_BVNodes[index].rightChildID;
                        if (leftID < m_BVNodes.size())
                                queue.push(
                                    make_pair(m_BVNodes[leftID], leftID));
                        if (rightID < m_BVNodes.size())
                                queue.push(
                                    make_pair(m_BVNodes[rightID], rightID));
                } 
    }  

}



LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE: {
        // 화면 해상도가 바뀌면 SwapChain을 다시 생성
        int width = int(LOWORD(lParam));
        int height = int(HIWORD(lParam));
        //cout << "width : " << width << "height : " << height << endl; 
        // 윈도우가 Minimize 모드에서는 screenWidth/Height가 0

        ResizeSwapChain(width, height);
    }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE: {
                    int currX =  LOWORD(lParam);
                    int currY =   HIWORD(lParam);
                    OnMouseMove(LOWORD(lParam), HIWORD(lParam));
                if (m_camera->m_isCameraLock == false) {

                        float addYaw = (float)(currX - m_preMouse[0]) * 0.0003f;
                        float addPitch = (float)(currY - m_preMouse[1]) * 0.001f;

                        if (std::abs(currX - m_preMouse[0]) > 10000)
                                addYaw = 0.0f;
                        if (std::abs(currY - m_preMouse[1]) > 10000)
                                addPitch = 0.0f;

                        m_camera->RotateCamera(addYaw, addPitch);
                }
                    m_preMouse[0] = currX;
                    m_preMouse[1] = currY;
    
    }
        break;
    case WM_LBUTTONDOWN: {

        m_inputManager->InputLeftMouse(true, LOWORD(lParam), HIWORD(lParam));
    }
        break;
    case WM_LBUTTONUP:
        m_inputManager->InputLeftMouse(false);
        break;
    case WM_RBUTTONDOWN:
        m_inputManager->InputRightMouse(true, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_RBUTTONUP:
        m_inputManager->InputRightMouse(false);
        break;
    case WM_KEYDOWN:
        m_inputManager->InputKey(true, wParam);

        if (wParam == VK_ESCAPE) { // ESC키 종료
            DestroyWindow(hwnd);
        }

        break;
    case WM_KEYUP:
        m_inputManager->InputKey(false, wParam);


        break;
    case WM_MOUSEWHEEL: {
        float m_wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        m_inputManager->InputMouseWheel(m_wheelDelta);
    }
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

void AppBase::ResizeSwapChain(int width, int height) {


    if (m_swapChain) {
        m_screenWidth = width;
        m_screenHeight = height;

        // 윈도우가 Minimize 모드에서는 screenWidth/Height가 0
        if (m_screenWidth && m_screenHeight) {

          //  cout << "Resize SwapChain to " << m_screenWidth << " "
              //   << m_screenHeight << endl;

            m_backBufferRTV.Reset();
            m_swapChain->ResizeBuffers(0, // 현재 개수 유지
                                       (UINT)m_screenWidth, // 해상도 변경
                                       (UINT)m_screenHeight,
                                       DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                                       0);

            m_camera->SetAspectRatio(this->GetAspectRatio());
            CreateBuffers();
            SetMainViewport();

            m_postProcess.Initialize(
                m_device, m_context, {m_postEffectsSRV, m_prevSRV},
                {m_backBufferRTV}, m_screenWidth, m_screenHeight, 4);
        }
    }


        
}

void AppBase::PostRender() {

    // Resolve MSAA texture
    m_context->ResolveSubresource(m_resolvedBuffer.Get(), 0,
                                  m_floatBuffer.Get(), 0,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);

    // PostEffects (m_globalConstsGPU 사용)
    AppBase::SetMainViewport();
    AppBase::SetPipelineState(Graphics::postEffectsPSO);
    AppBase::SetGlobalConsts(m_globalConstsGPU);
    vector<ID3D11ShaderResourceView *> postEffectsSRVs = {m_resolvedSRV.Get(),
                                                          m_depthOnlySRV.Get()};
    m_context->PSSetShaderResources(20, // 주의: Startslop 20
                                    UINT(postEffectsSRVs.size()),
                                    postEffectsSRVs.data());
    m_context->OMSetRenderTargets(1, m_postEffectsRTV.GetAddressOf(), NULL);
    m_context->PSSetConstantBuffers(5, // register(b5)
                                    1, m_postEffectsConstsGPU.GetAddressOf());
    m_screenSquare->Render(m_context);

    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    m_context->PSSetShaderResources(20, 2, nulls);

    // 후처리 (블룸 같은 순수 이미지 처리)
    AppBase::SetPipelineState(Graphics::postProcessingPSO);
    m_postProcess.Render(m_context);

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    m_context->CopyResource(
        m_prevBuffer.Get(),
        backBuffer.Get()); // 모션 블러 효과를 위해 렌더링 결과 보관
}

void AppBase::InitCubemaps(wstring basePath, wstring envFilename,
                           wstring specularFilename, wstring irradianceFilename,
                           wstring brdfFilename) {

    // BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
    D3D11Utils::CreateDDSTexture(m_device, (basePath + envFilename).c_str(),
                                 true, m_envSRV);
    D3D11Utils::CreateDDSTexture(
        m_device, (basePath + specularFilename).c_str(), true, m_specularSRV);
    D3D11Utils::CreateDDSTexture(m_device,
                                 (basePath + irradianceFilename).c_str(), true,
                                 m_irradianceSRV);
    D3D11Utils::CreateDDSTexture(m_device, (basePath + brdfFilename).c_str(),
                                 false, m_brdfSRV);
} 

// 여러 물체들이 공통적으료 사용하는 Const 업데이트
void AppBase::UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                                    const Matrix &viewRow,
                                    const Matrix &projRow, const Matrix &refl) {

       for (int i = 0; i < MAX_LIGHTS; i++) 
        m_shadowGlobalConstsCPU[i].globalTime += dt;
         
    m_globalConstsCPU.globalTime += dt;
    m_globalConstsCPU.eyeWorld = eyeWorld; 
    m_globalConstsCPU.cameraWorld = eyeWorld;
    m_globalConstsCPU.view = viewRow.Transpose();

    m_globalConstsCPU.proj = projRow.Transpose();
    m_globalConstsCPU.invProj = projRow.Invert().Transpose();
    m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    m_globalConstsCPU.invView = viewRow.Invert().Transpose();

    // 그림자 렌더링에 사용
    m_globalConstsCPU.invViewProj = m_globalConstsCPU.viewProj.Invert();

    m_reflectGlobalConstsCPU = m_globalConstsCPU;
    memcpy(&m_reflectGlobalConstsCPU, &m_globalConstsCPU, 
           sizeof(m_globalConstsCPU)); 
     
    m_reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
     
    Vector3 temp = m_reflectRow.Translation(); 
   m_reflectRow.Translation(Vector3());
    m_reflectGlobalConstsCPU.reflectWorld = m_reflectRow.Transpose();
   m_reflectRow.Translation(temp);
    
    m_reflectGlobalConstsCPU.viewProj = (refl * viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    m_reflectGlobalConstsCPU.invViewProj =
        m_reflectGlobalConstsCPU.viewProj.Invert();

    D3D11Utils::UpdateBuffer(m_context, m_globalConstsCPU, m_globalConstsGPU);
    D3D11Utils::UpdateBuffer(m_context, m_reflectGlobalConstsCPU,
                             m_reflectGlobalConstsGPU);
}
  
void AppBase::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0)
    m_context->VSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->HSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->DSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->GSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());

}
   
void AppBase::CreateDepthBuffers() {

    // DepthStencilView 만들기
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_screenWidth ;
    desc.Height = m_screenHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if (m_useMSAA && m_numQualityLevels > 0) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateDepthStencilView(
        depthStencilBuffer.Get(), NULL, m_defaultDSV.GetAddressOf()));

    // Depth 전용
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_depthOnlyBuffer.GetAddressOf()));

    // 그림자 Buffers (Depth 전용)
    desc.Width = m_shadowWidth;
    desc.Height = m_shadowHeight;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateTexture2D(
            &desc, NULL, m_shadowBuffers[i].GetAddressOf()));
    }
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                              m_overallShadowBuffer.GetAddressOf()));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(m_device->CreateDepthStencilView(
        m_depthOnlyBuffer.Get(), &dsvDesc, m_depthOnlyDSV.GetAddressOf()));

    // 그림자 DSVs
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(
            m_device->CreateDepthStencilView(m_shadowBuffers[i].Get(), &dsvDesc, m_shadowDSVs[i].GetAddressOf()));
    }
    ThrowIfFailed(
        m_device->CreateDepthStencilView(m_overallShadowBuffer.Get(), &dsvDesc, m_overallShadowDSV.GetAddressOf()));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_depthOnlyBuffer.Get(), &srvDesc, m_depthOnlySRV.GetAddressOf()));

    // 그림자 SRVs
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateShaderResourceView(
            m_shadowBuffers[i].Get(), &srvDesc,
            m_shadowSRVs[i].GetAddressOf()));
    }
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_overallShadowBuffer.Get(), &srvDesc,
        m_overallShadowSRV.GetAddressOf()));
} 
 
void AppBase::SetPipelineState(const GraphicsPSO &pso) {

    m_context->VSSetShader(pso.m_vertexShader.Get(), 0, 0); 
    m_context->PSSetShader(pso.m_pixelShader.Get(), 0, 0);
    m_context->HSSetShader(pso.m_hullShader.Get(), 0, 0);
    m_context->DSSetShader(pso.m_domainShader.Get(), 0, 0);
    m_context->GSSetShader(pso.m_geometryShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);
    m_context->IASetInputLayout(pso.m_inputLayout.Get());
    m_context->RSSetState(pso.m_rasterizerState.Get());
    m_context->OMSetBlendState(pso.m_blendState.Get(), pso.m_blendFactor,
                               0xffffffff);
    m_context->OMSetDepthStencilState(pso.m_depthStencilState.Get(),
                                      pso.m_stencilRef);
    m_context->IASetPrimitiveTopology(pso.m_primitiveTopology);
}

void AppBase::SetPipelineState(const ComputePSO &pso) {
    m_context->VSSetShader(NULL, 0, 0);
    m_context->PSSetShader(NULL, 0, 0);
    m_context->HSSetShader(NULL, 0, 0);
    m_context->DSSetShader(NULL, 0, 0);
    m_context->GSSetShader(NULL, 0, 0);
    m_context->CSSetShader(pso.m_computeShader.Get(), 0, 0);
}

shared_ptr<Model> AppBase::PickClosest(const Ray &pickingRay, float &minDist) {
    minDist = 1e5f;
    shared_ptr<Model> minModel = nullptr;
    for (auto &model : m_basicList) {
        float dist = 0.0f;
        if (model->m_isPickable &&
            pickingRay.Intersects(model->m_boundingSphere, dist) &&
            dist < minDist) {
            minModel = model;
            minDist = dist;
        }
    }
    return minModel;
}

void AppBase::ProcessMouseControl() {

    static shared_ptr<Model> activeModel = nullptr;
    static float prevRatio = 0.0f;
    static Vector3 prevPos(0.0f);
    static Vector3 prevVector(0.0f);

    // 적용할 회전과 이동 초기화
    Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 dragTranslation(0.0f);
    Vector3 pickPoint(0.0f);
    float dist = 0.0f;

    // 사용자가 두 버튼 중 하나만 누른다고 가정합니다.
    if (m_leftButton || m_rightButton) {
        const Matrix viewRow = m_camera->GetViewRow();
        const Matrix projRow = m_camera->GetProjRow();
        const Vector3 ndcNear = Vector3(m_mouseNdcX, m_mouseNdcY, 0.0f);
        const Vector3 ndcFar = Vector3(m_mouseNdcX, m_mouseNdcY, 1.0f);
        const Matrix invProjView = (viewRow * projRow).Invert();
        const Vector3 worldNear = Vector3::Transform(ndcNear, invProjView);
        const Vector3 worldFar = Vector3::Transform(ndcFar, invProjView);
        Vector3 dir = worldFar - worldNear;
        dir.Normalize();
        const Ray curRay = SimpleMath::Ray(worldNear, dir);

        // 이전 프레임에서 아무 물체도 선택되지 않았을 경우에는 새로 선택
        if (!activeModel) {
            auto newModel = AppBase::PickClosest(curRay, dist);
            if (newModel) {
                cout << "Newly selected model: " << newModel->m_name << endl;
                activeModel = newModel;
                m_pickedModel = newModel; // GUI 조작용 포인터
                pickPoint = curRay.position + dist * curRay.direction;
                if (m_leftButton) { // 왼쪽 버튼 회전 준비
                    prevVector =
                        pickPoint - activeModel->m_boundingSphere.Center;
                    prevVector.Normalize();
                } else { // 오른쪽 버튼 이동 준비
                    m_dragStartFlag = false;
                    prevRatio = dist / (worldFar - worldNear).Length();
                    prevPos = pickPoint;
                }
            }
        } else {                // 이미 선택된 물체가 있었던 경우
            if (m_leftButton) { // 왼쪽 버튼으로 계속 회전
                if (curRay.Intersects(activeModel->m_boundingSphere, dist)) {
                    pickPoint = curRay.position + dist * curRay.direction;
                } else {
                    // 바운딩 스피어에 가장 가까운 점을 찾기
                    Vector3 c =
                        activeModel->m_boundingSphere.Center - worldNear;
                    Vector3 centerToRay = dir.Dot(c) * dir - c;
                    pickPoint =
                        c +
                        centerToRay *
                            std::clamp(activeModel->m_boundingSphere.Radius /
                                           centerToRay.Length(),
                                       0.0f, 1.0f);
                    pickPoint += worldNear;
                }

                Vector3 currentVector =
                    pickPoint - activeModel->m_boundingSphere.Center;
                currentVector.Normalize();
                float theta = acos(prevVector.Dot(currentVector));
                if (theta > 3.141592f / 180.0f * 3.0f) {
                    Vector3 axis = prevVector.Cross(currentVector);
                    axis.Normalize();
                    q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                                                                    theta);
                    prevVector = currentVector;
                }

            } else { // 오른쪽 버튼으로 계속 이동
                Vector3 newPos = worldNear + prevRatio * (worldFar - worldNear);
                if ((newPos - prevPos).Length() > 1e-3) {
                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
                pickPoint = newPos; // Cursor sphere 그려질 위치
            }
        }
    } else {
        // 버튼에서 손을 땠을 경우에는 움직일 모델은 nullptr로 설정
        activeModel = nullptr;

        // m_pickedModel은 GUI 조작을 위해 마우스에서 손을 떼도 nullptr로
        // 설정하지 않음
    }
    //pitch = std::atan2(-rotationMatrix[13],
    //                   std::sqrt(rotationMatrix[23] * rotationMatrix[23] +
    //                             rotationMatrix[33] * rotationMatrix[33]));

    // Cursor sphere 그리기
    if (activeModel) {
        Vector3 translation = activeModel->m_worldRow.Translation();
        activeModel->m_worldRow.Translation(Vector3(0.0f));


        Matrix tempRow =
            activeModel->m_worldRow * Matrix::CreateFromQuaternion(q);


        //activeModel->UpdateWorldRow(
        //    activeModel->m_worldRow *
        //        Matrix::CreateFromQuaternion(q) *
        //    Matrix::CreateTranslation(dragTranslation + translation));

        Matrix temp = activeModel->m_worldRow *
                      Matrix::CreateFromQuaternion(q) *
                      Matrix::CreateTranslation(dragTranslation + translation);

        //activeModel->UpdateWorldRow(temp);

        Vector3 tempAngle, tempPos, tempScale;
        Model::ExtractEulerAnglesFromMatrix(&temp, tempAngle);
        Model::ExtractPositionFromMatrix(&temp, tempPos);
        Model::ExtractScaleFromMatrix(&temp, tempScale);
        //activeModel->UpdateTranseform(activeModel->GetScale(),
        //                              Vector3(pitch, yaw, pitch),
        //                              dragTranslation + translation);
        activeModel->UpdateTranseform(tempScale, tempAngle, tempPos);

        activeModel->m_boundingSphere.Center =
            activeModel->m_worldRow.Translation();

        // 충돌 지점에 작은 구 그리기
        m_cursorSphere[0]->m_isVisible = true;
        //m_cursorSphere->UpdateWorldRow(Matrix::CreateTranslation(pickPoint));
        m_cursorSphere[0]->UpdatePosition(pickPoint);
    } else {
        m_cursorSphere[0]->m_isVisible = false;
    }
}

bool AppBase::InitMainWindow() {

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

    if (!RegisterClassEx(&wc)) {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    RECT wr = {0, 0, m_screenWidth, m_screenHeight};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
    m_mainWindow = CreateWindow(wc.lpszClassName, L"HongLabGraphics Example",
                                WS_OVERLAPPEDWINDOW,
                                100, // 윈도우 좌측 상단의 x 좌표
                                100, // 윈도우 좌측 상단의 y 좌표
                                wr.right - wr.left, // 윈도우 가로 방향 해상도
                                wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                                NULL, NULL, wc.hInstance, NULL);

    if (!m_mainWindow) {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_mainWindow);

    return true;
}

bool AppBase::InitDirect3D() {

    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
    // const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format = m_backBufferFormat;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT |
                     DXGI_USAGE_UNORDERED_ACCESS; // Compute Shader
    sd.OutputWindow = m_mainWindow;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가
    // 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(), &featureLevel, m_context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        cout << "D3D Feature Level 11 unsupported." << endl;
        return false;
    }

    Graphics::InitCommonStates(m_device);

    CreateBuffers();

    SetMainViewport();

    // 공통으로 쓰이는 ConstBuffers
    D3D11Utils::CreateConstBuffer(m_device, m_globalConstsCPU,
                                  m_globalConstsGPU);
    D3D11Utils::CreateConstBuffer(m_device, m_reflectGlobalConstsCPU,
                                  m_reflectGlobalConstsGPU);

    // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
    for (int i = 0; i < MAX_LIGHTS; i++) {
        D3D11Utils::CreateConstBuffer(m_device, m_shadowGlobalConstsCPU[i],
                                      m_shadowGlobalConstsGPU[i]);
    }

    // 후처리 효과용 ConstBuffer
    D3D11Utils::CreateConstBuffer(m_device, m_postEffectsConstsCPU,
                                  m_postEffectsConstsGPU);

    return true;
}

bool AppBase::InitGUI() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_mainWindow)) {
        return false;
    }

    return true;
}
 
void AppBase::SetMainViewport() {



        // Set the viewport
        ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));

        m_screenViewport.TopLeftX = 0;
        m_screenViewport.TopLeftY = 0;
        m_screenViewport.Width =
            float(m_screenWidth);
        m_screenViewport.Height = float(m_screenHeight);
        m_screenViewport.MinDepth = 0.0f;
        m_screenViewport.MaxDepth = 1.0f;
      
       // std::cout << "m_imGuiWidth" << m_imGuiWidth << std::endl;

        m_context->RSSetViewports(1, &m_screenViewport);
    
}

void AppBase::SetShadowViewport() {

    // Set the viewport
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX =0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = float(m_shadowWidth);
    shadowViewport.Height = float(m_shadowHeight);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &shadowViewport);
}
 
void AppBase::ComputeShaderBarrier() {

    // 참고: BreadcrumbsDirectX-Graphics-Samples (DX12)
    // void CommandContext::InsertUAVBarrier(GpuResource & Resource, bool
    // FlushImmediate) 
          
    // 예제들에서 최대 사용하는 SRV, UAV 갯수가 6개
    ID3D11ShaderResourceView *nullSRV[6] = {
        0, 
    }; 
    m_context->CSSetShaderResources(0, 6, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[6] = { 
        0, 
    };
    m_context->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
}
 
void AppBase::UpdateBVH() { 
        m_BVNodes.clear();
        if (m_basicList.size() == 0)  
                return; 
         //              0                      9  
        //        0            4          4          9 
        //    0     2     2    4   4     6    6     9
        //  0 1  12   23  34 45  56  67    79
        //                                                 7889 
        auto CreateBVNode = [&](int min, int max) 
        { 
                 
                BoundingBox enclosingBox = m_basicList[min]->m_boundingBox;
                    Matrix& Box1Row = m_basicList[min]->m_worldRow;
                    Vector3 box1center = enclosingBox.Center;
                    Vector3 box1Extents = enclosingBox.Extents;

                    vector<Vector3> box1Corners = {
                        box1center + Vector3(-1.0f, -1.0f, -1.0f) * box1Extents,
                        box1center + Vector3(-1.0f, 1.0f, -1.0f) * box1Extents,
                        box1center + Vector3(1.0f, 1.0f, -1.0f) * box1Extents,
                        box1center + Vector3(1.0f, -1.0f, -1.0f) * box1Extents,
                        box1center + Vector3(-1.0f, -1.0f, 1.0f) * box1Extents,
                        box1center + Vector3(-1.0f, 1.0f, 1.0f) * box1Extents,
                        box1center + Vector3(1.0f, 1.0f, 1.0f) * box1Extents,
                        box1center + Vector3(1.0f, -1.0f, 1.0f) * box1Extents};
                    for (int i = 0; i < box1Corners.size(); i++) {
                        box1Corners[i] = Vector3::Transform(box1Corners[i], Box1Row);
                    }
                     
                    Vector3 minCorner = box1Corners[0];
                    Vector3 maxCorner = box1Corners[0];
                    for (int i = 1; i < box1Corners.size(); i++) 
                    {
                        minCorner = Vector3::Min(minCorner, box1Corners[i]);
                        maxCorner = Vector3::Max(maxCorner, box1Corners[i]);

                    }   
                      
                for (int i = min + 1; i < max; i++) 
                {
                        Matrix& Box2Row = m_basicList[i]->m_worldRow;
            
                        Vector3 box2center =
                            m_basicList[i]->m_boundingBox.Center;
                        Vector3 box2Extents =
                            m_basicList[i]->m_boundingBox.Extents;
                                  
                        vector<Vector3> box2Corners = {
                                box2center + Vector3(-1.0f, -1.0f, -1.0f) *
                                                box2Extents,
                                box2center + Vector3(-1.0f, 1.0f, -1.0f) *
                                                box2Extents,
                                box2center + Vector3(1.0f, 1.0f, -1.0f) *
                                                box2Extents, 
                                box2center + Vector3(1.0f, -1.0f, -1.0f) *
                                                box2Extents,
                                box2center + Vector3(-1.0f, -1.0f, 1.0f) *
                                                box2Extents,
                                box2center + Vector3(-1.0f, 1.0f, 1.0f) *
                                                box2Extents,
                                box2center + Vector3(1.0f, 1.0f, 1.0f) * 
                                                box2Extents,
                                box2center + Vector3(1.0f, -1.0f, 1.0f) *
                                                box2Extents
                        }; 
                        for (int j = 0; j < box2Corners.size(); j++) {
                                box2Corners[j] = Vector3::Transform(
                                box2Corners[j], Box2Row); 
                        } 
                                  
                        for (int j = 0; j < box2Corners.size(); j++) {
                               minCorner = Vector3::Min(minCorner, box2Corners[j]);
                                maxCorner = Vector3::Max(maxCorner, box2Corners[j]);

                        } 
                           
                }  
                enclosingBox.Center = (minCorner + maxCorner) * 0.5f;
                enclosingBox.Extents = (maxCorner - minCorner) * 0.5f;

                BVNode result = BVNode(enclosingBox);
                if (max - min == 1) {
                        result.objectID = min;  
                }
         //       cout << "minMax : " << min << ", " << max << "\n";  
                    
                /*auto meshData = GeometryGenerator::MakeWireBox(
                    enclosingBox.Center, enclosingBox.Extents + Vector3(1e-3f));

                m_BVNMeshs.push_back(std::make_shared<Mesh>());
                D3D11Utils::CreateVertexBuffer(m_device, meshData.vertices,
                                               m_BVNMeshs.back()->vertexBuffer);
                m_BVNMeshs.back()->indexCount = UINT(meshData.indices.size());
                m_BVNMeshs.back()->vertexCount = UINT(meshData.vertices.size());
                m_BVNMeshs.back()->stride = UINT(sizeof(Vertex));
                  
                ConstantBuffer<MeshConstants> tempMeshCst;
                tempMeshCst.GetCpu().world = Matrix();
                tempMeshCst.GetCpu().world.Transpose();  
                tempMeshCst.Initialize(m_device);
                ConstantBuffer<MaterialConstants> tempMaterialCst;
                tempMaterialCst.Initialize(m_device);*/
                 

                //m_BVNMeshs.back()->meshConstsGPU = tempMeshCst.Get();
                //m_BVNMeshs.back()->materialConstsGPU = tempMaterialCst.Get();
                  
                return result;
        };
          
        std::queue<std::pair<int, int>> queue;
        queue.push(make_pair(0, m_basicList.size()));
             
        int index = 0;
        while (!queue.empty()) 
        {  
                int min = queue.front().first;
                int max = queue.front().second;
                queue.pop();  
                m_BVNodes.push_back(CreateBVNode(min, max)); 
                
                int midValue = (max + min) / 2; 
                int nextMin = min; 
                int nextMax = midValue;
                if (nextMax - nextMin > 0 && nextMax != max) {
                        queue.push(make_pair(nextMin, nextMax));
                        m_BVNodes.back().leftChildID = ++index;
                }
                  
                nextMin = midValue;  
                nextMax = max; 
                if (nextMax - nextMin > 0 && nextMin != min) {
                        queue.push(make_pair(nextMin, nextMax)); 
                        m_BVNodes.back().rightChildID = ++index;
                } 
        }
         
          
        cout << "m_basicList Size : " << m_basicList.size() << "\n";
}


void AppBase::replicateObject()
{ 
        if (m_pickedModel == nullptr)
                return;

        ObjectSaveInfo temp = m_pickedModel->objectInfo;
        temp.scale = m_pickedModel->GetScale();
        temp.rotation = m_pickedModel->GetRotation();
        temp.position = m_pickedModel->GetPosition();
        temp.metallic = m_pickedModel->m_materialConsts.GetCpu().metallicFactor;
        temp.roughness = m_pickedModel->m_materialConsts.GetCpu().roughnessFactor;
        temp.minMetallic = m_pickedModel->m_materialConsts.GetCpu().minMetallic;
        temp.minRoughness =
            m_pickedModel->m_materialConsts.GetCpu().minRoughness;

        m_JsonManager->CreateMesh(temp);
}

void AppBase::AddBasicList(shared_ptr<Model>& object, bool isBasicList, bool editable, bool saveable) {
         
        if (object == nullptr)
                return;
        object->m_editable = editable;
        object->m_saveable = saveable;

        static int indexID = 1;
        int id_R = 0, id_G = 0, id_B = 0, id_A = 0;
        id_R = indexID % 256;

        if (indexID > 255)
                id_G = (indexID / 256) % 256;

        if (indexID > 65536)
                id_B = (indexID / 65536) % 256;

        object->objectInfo.objectID = indexID;
        object->m_meshConsts.GetCpu().indexColor[0] = (float)id_R / 255;
        object->m_meshConsts.GetCpu().indexColor[1] = (float)id_G / 255;
        object->m_meshConsts.GetCpu().indexColor[2] = (float)id_B / 255;

        if (isBasicList)
                m_basicList.push_back(object);
        else 
                m_NoneBVHList.push_back(object);

        m_objects.insert(make_pair(indexID, object));

        indexID++;
}



bool AppBase::IsMouseHoveringImGui() { 
        

        bool inX = m_mouseX > ImPos.x &&
                   m_mouseX < ImPos.x + ImSize.x;
        bool inY = m_mouseY > ImPos.y &&
                   m_mouseY < ImPos.y + ImSize.y;

        if (inX && inY) {
                return true;
        }
        return false;
} 

    template <typename T>
bool AppBase::ReadPixelOfMousePos(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context) {
    D3D11_TEXTURE2D_DESC desc;
    m_indexTempTexture->GetDesc(&desc);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // cpu에서 읽기 가능. 
    desc.Usage = D3D11_USAGE_STAGING;
    ThrowIfFailed(device->CreateTexture2D(
        &desc, NULL, m_indexStagingTexture.GetAddressOf()));
    
    D3D11_BOX box;
    box.left = std::clamp(m_mouseX  - 1, 0, 
                          (int)desc.Width - 1);
    box.right = box.left + 1;
    box.top = std::clamp(m_mouseY - 1, 0, (int)desc.Height - 1);
    box.bottom = box.top + 1;
    box.front = 0;
    box.back = 1;

   //D3D11_BOX box;
   // box.left = 0;
   // box.right = desc.Width;
   // box.top = 0;
   // box.bottom = desc.Height;
   // box.front = 0;
   // box.back = 1; 

    //cout << "mouse XY : " << box.left << " " << box.top << endl;
    context->CopySubresourceRegion(m_indexStagingTexture.Get(), 0, 0, 0, 0, m_indexTempTexture.Get(), 0, &box);

    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(m_indexStagingTexture.Get(), NULL, D3D11_MAP_READ, NULL, &ms);

    std::vector<T> pixels(desc.Width * desc.Height * 4);

    T *pData = (T *)ms.pData;

    context->Unmap(m_indexStagingTexture.Get(), NULL);
     
   
     cout << "color : " << (int)pData[0] << " " << (int)pData[1] << " "
         << (int)pData[2] << " " << (int)pData[3] << endl; 
      int objID = (int)pData[0] + (int)pData[1] * 256 +
                 (int)pData[2] * 65536; //+ (int)pData[3];
    auto object = m_objects.find(objID);
    //  cout << "object ID : " << objID << endl; 
    
    if (object!= m_objects.end()) 
    {
        shared_ptr<Model> tempObj = object->second;
        float tempID = tempObj->objectInfo.objectID;
        string tempName = tempObj->objectInfo.meshName;
        //std::cout << "Selected [" << tempName << "] Object ID : " << tempID << std::endl;

        //if (m_pickedModel == tempObj && m_keyPressed['Q'] == false) 
        //{
        //    m_pickedModel->m_materialConsts.GetCpu().isSelected = 0;
        //    m_pickedModel = nullptr;
        //    return false;
        //}

        if (m_pickedModel != nullptr) 
        {
                m_pickedModel->m_materialConsts.GetCpu().isSelected = 0;
        }
        m_pickedModel = tempObj;
        m_pickedModel->m_materialConsts.GetCpu().isSelected = 1;

        return true;
                
    } 
    else 
    {
        if (m_pickedModel != nullptr) {
                m_pickedModel->m_materialConsts.GetCpu().isSelected = 0;
                m_pickedModel = nullptr;
        }
        return false;
    }

}


void AppBase::CreateBuffers() {

    // 레스터화 -> float/depthBuffer(MSAA) -> resolved -> backBuffer
    // BackBuffer는 화면으로 최종 출력 (SRV는 불필요)
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        backBuffer.Get(), NULL, m_backBufferRTV.GetAddressOf()));

    // FLOAT MSAA RenderTargetView/ShaderResourceView
    ThrowIfFailed(m_device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &m_numQualityLevels));

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // 이전 프레임 저장용
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_prevBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_prevBuffer.Get(), NULL,  m_prevRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(m_prevBuffer.Get(), NULL,  m_prevSRV.GetAddressOf()));

    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    //desc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
    //desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
  //  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;

    //desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (m_useMSAA && m_numQualityLevels) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }

    // -        -       -       -       -       -       -       -       -       -       -       -       
    

    // -        -       -       -       -       -       -       -       -       -       -       -       
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL, m_floatBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_floatBuffer.Get(), NULL,m_floatRTV.GetAddressOf()));

    D3D11_TEXTURE2D_DESC descTemp;
    descTemp = desc;
    descTemp.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    // backBuffer->GetDesc(&desc);  
    ThrowIfFailed(m_device->CreateTexture2D(&descTemp, NULL,
                                                  m_indexTexture.GetAddressOf())); 
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_indexTexture.Get(), NULL, m_indexRenderTargetView.GetAddressOf()));
    //desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

      
    //D3D11_TEXTURE2D_DESC desc3;
    //backBuffer->GetDesc(&desc3); 
    //cout << "cpuAccesccFlag" << desc3.CPUAccessFlags << endl; 

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV 
    desc.SampleDesc.Count = 1; 
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
                     D3D11_BIND_UNORDERED_ACCESS;
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_resolvedBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateTexture2D( 
        &desc, NULL, m_postEffectsBuffer.GetAddressOf()));

    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = 0;

       
    // HDR
    {
        D3D11_TEXTURE2D_DESC desc2;
        backBuffer->GetDesc(&desc2);
        // 디버깅용 
        // cout << desc.Width << " " << desc.Height << " " << desc.Format <<
        // endl;
        desc2.SampleDesc.Count = 1;
        desc2.SampleDesc.Quality = 0;
        desc2.BindFlags = D3D11_BIND_RENDER_TARGET |D3D11_BIND_SHADER_RESOURCE;
        desc2.MiscFlags = 0; 
        //desc2.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
       // desc2.Format = DXGI_FORMAT_R16G16B16A16_UINT;
        desc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        
        ThrowIfFailed( 
        m_device->CreateTexture2D(&desc2, NULL,  m_tempTexture.GetAddressOf()));
         
        ThrowIfFailed(m_device->CreateTexture2D(
                &desc2, NULL, m_indexTempTexture.GetAddressOf()));

        desc2.BindFlags = 0;
        desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc2.Usage = D3D11_USAGE_STAGING;
        desc2.Width = 1;
        desc2.Height = 1;
        ThrowIfFailed(m_device->CreateTexture2D( 
            &desc2, nullptr, m_indexStagingTexture.GetAddressOf()));

    } 
    //desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;



    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_resolvedBuffer.Get(), NULL, m_resolvedSRV.GetAddressOf()));

    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_postEffectsBuffer.Get(), NULL, m_postEffectsSRV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_resolvedBuffer.Get(), NULL, m_resolvedRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_postEffectsBuffer.Get(), NULL, m_postEffectsRTV.GetAddressOf()));
    CreateDepthBuffers();

    m_postProcess.Initialize(m_device, m_context, {m_postEffectsSRV, m_prevSRV},
                             {m_backBufferRTV}, m_screenWidth, m_screenHeight,
                             4);

        std::vector<std::string> filenames = {
        "../../Assets/Textures/TreeBillboards/1.png",
        "../../Assets/Textures/TreeBillboards/2.png",
        "../../Assets/Textures/TreeBillboards/3.png",
        "../../Assets/Textures/TreeBillboards/4.png",
        "../../Assets/Textures/TreeBillboards/5.png",
        //"../../Assets/Textures/TreeBillboards/6.png"
        }; 

    D3D11Utils::CreateTextureArray(m_device, m_context,
                                   filenames, m_texArray,
                                   m_billboardTreeSRV);


}

} // namespace hlab