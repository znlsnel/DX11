#include "Ex2001_GamePlay.h"

#include "BillboardModel.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "OceanModel.h"
#include "Character.h"
#include "AppBase.h"
#include "JsonManager.h"
#include "TessellationModel.h"
#include "GrassModel.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex2001_GamePlay::Ex2001_GamePlay() : AppBase() {}

bool Ex2001_GamePlay::Initialize() {
    bool returnValue = AppBase::Initialize();
    auto it = m_JsonManager->quicellPaths.find(
        "C:\\DEVELOPMENT\\GIT\\DX11_HongLab\\Assets\\Quicells\\q49\\");
    if ( false && it != m_JsonManager->quicellPaths.end()) {

        int meshSize = it->second.mesh.size();
        std::mt19937 gen; 
        std::uniform_real_distribution<float> randId(0.0f, float(meshSize) - 1.0f);
        std::uniform_real_distribution<float> randRange(-0.1f, 0.1f); 
        std::uniform_real_distribution<float> randRot(-3.141592f, 3.141592f); 
         
        // -30  ~ 30 60 * 60 = 3600 / 1000
        // 60 * 60        
        int count = 1000;
        float dx = 3600.f / float(count);
        for (int i = 0; i < count; i++) {
                float dxI = dx * i;
                float posZ = dxI / 60;
                float posX = (dxI - std::roundf(dxI)) + int(dxI) % 60;
                Vector3 tempPos = Vector3(posX , 3.0f, posZ);
              const int meshID = int(randId(gen));    
               const float randX = randRange(gen);
              const float randZ = randRange(gen); 
                const float randYaw = randRot(gen); 
                 
               ObjectSaveInfo tempInfo;
               tempInfo.quicellPath = "C:\\DEVELOPMENT\\GIT\\DX11_"
                                      "HongLab\\Assets\\Quicells\\q49\\";
               tempInfo.meshID = -2;
               tempInfo.rotation =
                   Vector3(3.141592f * 90.f / 180.f, randYaw, 0.0f); 
               tempInfo.minMetallic = 0.5f;
               tempInfo.minRoughness = 1.0f;
               tempInfo.position = tempPos; 
                  
               float dist = 0.0f;
               SetHeightPosition(tempInfo.position + Vector3(0.0f, 5.0f, 0.0f),
                                 Vector3(0.0f, -1.0f, 0.0f), dist);
               if (dist > 0.0f) 
                   tempInfo.position = tempInfo.position +
                                       Vector3(0.0f, 5.0f, 0.0f) +
                                   Vector3(0.0f, -1.0f, 0.0f) * dist;

               shared_ptr<Model> tempModel =
                   m_JsonManager->CreateMesh(tempInfo);

               Matrix tempRow = tempModel->m_worldRow;
               tempRow.Translation(Vector3(0.0f));
               Vector3 tempExtents = Vector3::Transform( 
                   tempModel->m_boundingBox.Extents, tempRow);
               tempModel->UpdatePosition(tempModel->GetPosition() +
                                         Vector3(0.0f, tempExtents.y, 0.0f));
        }
    }
    return returnValue; 
}
 
bool Ex2001_GamePlay::InitScene() {

    AppBase::m_globalConstsCPU.strengthIBL = 0.5f;
    AppBase::m_globalConstsCPU.lodBias = 0.0f;

    AppBase::m_camera->Reset(Vector3(1.60851f, 0.409084f, 0.560064f), -1.65915f,
                            0.0654498f);

    AppBase::InitCubemaps(
        L"../Assets/Textures/Cubemaps/HDRI/Sky/", L"skyEnvHDR.dds",
        L"skySpecularHDR.dds", L"skyDiffuseHDR.dds",
        L"skyBrdf.dds");

    AppBase::InitScene(); 
     
    // 바닥(거울) 
     
     
    //InitPhysics(true);  
     
    InitAudio();  
           
    // Plane    
    if (true)
    {    
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
         
        Vector2 mapTexScale = Vector2(100.f, 100.f);
         int mapArea = 200;
        float mapScale = 30.f;
        auto mesh = GeometryGenerator::MakeTessellationPlane(
            mapArea, mapArea, mapScale, Vector2(100.0f, 100.0f),
             heightMapImage); 
        string path = "../Assets/Textures/PBR/TerrainTextures/Ground037_4K-PNG/";
        mesh.albedoTextureFilename =  path + "Ground037_4K-PNG_Color.png";
        mesh.aoTextureFilename=
            path + "Ground037_4K-PNG_AmbientOcclusion.png";
        mesh.normalTextureFilename = path + "Ground037_d4K-PNG_NormalDX.png";
        // mesh.roughnessTextureFilename = path + "Ground037_4K-PNG_Roughness.png";
        mesh.heightTextureFilename = 
            path + "Ground037_4K-PNG_Displacement.png";
                   
            m_groundPlane = 
                make_shared<TessellationModel>(
                m_device, m_context,  vector{mesh}, this, true);
        m_groundPlane->mapScale = mapScale;
            m_groundPlane->texScale = mapTexScale;
        m_groundPlane->mapArea[0] = mapArea; 
        m_groundPlane->mapArea[1] = mapArea; 

            m_groundPlane->m_materialConsts.GetCpu().albedoFactor =
                Vector3(0.2f);
            m_groundPlane->m_materialConsts.GetCpu().emissionFactor =
                Vector3(0.0f);
            m_groundPlane->m_materialConsts.GetCpu().metallicFactor = 0.f;
            m_groundPlane->m_materialConsts.GetCpu().roughnessFactor = 1.0f;
            m_groundPlane->m_materialConsts.GetCpu().useNormalMap = 1;
            m_groundPlane->UpdateRotation(
                Vector3(90 * 3.141592f / 180.f, 0.0f, 0.0f)); 
            shared_ptr<Model> temp = m_groundPlane;
            temp->isObjectLock = true;  
            AddBasicList(temp, false);  
    }
       
         //Grass object  
    if (true)  
    {
            shared_ptr<GrassModel> grass = make_shared<GrassModel>();
            shared_ptr<Model> temp = static_pointer_cast<Model>(grass);
            AddBasicList(temp, true, true, false);
            // Instances 만들기 
            std::mt19937 gen(0); 
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            vector<GrassInstance> &grassInstances = grass->m_instancesCpu;   
            for (int i = 0; i < 10000; i++) 
            { 
                    const float lengthScale = dist(gen) * 0.7f + 0.3f;
                    const float widthScale = 0.5f + dist(gen) * 0.5f;
                    const Vector3 pos = Vector3(dist(gen) * 0.2f, 0.0f, dist(gen) * 0.2f) *  5.0f;
                    const float angle = dist(gen) * 3.141592f; // 바라보는 방향
                    const float slope = (dist(gen) - 0.5f) * 2.0f * 3.141592f * 0.2f; // 기본 기울기
                    GrassInstance gi;
                    gi.instanceWorld =
                        Matrix::CreateRotationX(slope) *
                        Matrix::CreateRotationY(angle) *
                        Matrix::CreateScale(widthScale* 0.3, lengthScale * 0.3f, 0.3f) *
                        Matrix::CreateTranslation(pos);  
                    gi.windStrength = 0.5f; 
                    grassInstances.push_back(gi);
            }
            // 쉐이더로 보내기 위해 transpose
            for (auto &i : grassInstances) {
            i.instanceWorld = i.instanceWorld.Transpose();
            }
            grass->Initialize(m_device, m_context);
            // m_grass->UpdateWorldRow(Matrix::CreateScale(0.5f) *
            //                         Matrix::CreateTranslation(0.0f,
            //                         0.0f, 2.0f));
    }
      

    // ocean 
        {
        auto mesh = GeometryGenerator::MakeSquare(200.0, {10.0f, 10.0f});
        m_ocean =
            make_shared<OceanModel>(m_device, m_context, vector{mesh});
        m_ocean->m_castShadow = false;

        Vector3 position = Vector3(0.0f, 0.3f, 2.0f);
        //m_ocean->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
        //                        Matrix::CreateTranslation(position));
        m_ocean->UpdateTranseform(m_ocean->GetScale(),
                                  Vector3(3.141592f * 0.5f,
                                          m_ocean->GetRotation().y,
                                          m_ocean->GetRotation().z),
                                  position);
        AddBasicList(m_ocean, false ); 
    }
        
    return true;
}

void Ex2001_GamePlay::CreateStack(const PxTransform &t, int numStacks,
                                  int numWidth, PxReal halfExtent) {

    vector<MeshData> box = {GeometryGenerator::MakeBox(halfExtent * 10)};
    
    PxShape *shape = gPhysics->createShape(
        PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);

    for (int i = 0; i < numStacks; i++) {
        for (int j = 0; j < numWidth - i; j++) {
            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(numWidth - i),
                                       PxReal(i * 2 + 1), 0) *
                                halfExtent);
            PxRigidDynamic *body =
                gPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
            gScene->addActor(*body);

            auto m_newObj = std::make_shared<Model>(
                m_device, m_context, box); // <- 우리 렌더러에 추가
            m_newObj->m_materialConsts.GetCpu().albedoFactor = Vector3(0.8f);
            //AppBase::m_basicList.push_back(m_newObj);
            AddBasicList(m_newObj);
            this->m_PhysicalObjects.push_back(m_newObj);
        }
    }
    shape->release();
}

void Ex2001_GamePlay::InitPhysics(bool interactive) {
    gFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport *transport =
        PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
                               PxTolerancesScale(), true, gPvd);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
                                   true);
    }
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic *groundPlane =
        PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);

    for (PxU32 i = 0; i < 5; i++)
        CreateStack(PxTransform(PxVec3(0, 0, stackZ -= 15.0f)), 8, 20, 2.5f);
}

PxRigidDynamic *Ex2001_GamePlay::CreateDynamic(const PxTransform &t,
                                               const PxGeometry &geometry,
                                               const PxVec3 &velocity) {

    m_fireball = std::make_shared<BillboardModel>(this, 2.0f);
    
     //m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
     //                       1.0f, L"GameExplosionPS.hlsl");
    Vector3 dir(velocity.x, velocity.y, velocity.z);
    dir.Normalize();
    m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
    m_fireball->m_castShadow = false;
    m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
                           0.2f, Graphics::volumetricFirePS);
   
    AppBase::m_basicList.push_back(m_fireball);
    this->m_PhysicalObjects.push_back(m_fireball);

    PxRigidDynamic *dynamic =
        PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    gScene->addActor(*dynamic);
    return dynamic;
}

void Ex2001_GamePlay::UpdateLights(float dt) { AppBase::UpdateLights(dt); }

void Ex2001_GamePlay::Update(float dt) {
    timeSeconds += dt;

    AppBase::Update(dt);

     
    // 이하 물리엔진 관련
    if (false)
    {

            gScene->simulate(1.0f / 60.0f);
            gScene->fetchResults(true);

            PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                                 PxActorTypeFlag::eRIGID_STATIC);
            std::vector<PxRigidActor *> actors(nbActors);
            gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                  PxActorTypeFlag::eRIGID_STATIC,
                              reinterpret_cast<PxActor **>(&actors[0]), nbActors);

            PxShape *shapes[MAX_NUM_ACTOR_SHAPES];

            int count = 0;

            for (PxU32 i = 0; i < nbActors; i++) {
                const PxU32 nbShapes = actors[i]->getNbShapes();
                PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
                actors[i]->getShapes(shapes, nbShapes);
                for (PxU32 j = 0; j < nbShapes; j++) {
                    const PxMat44 shapePose(
                        PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

                    if (actors[i]->is<PxRigidDynamic>()) {

                        // bool speeping = actors[i]->is<PxRigidDynamic>() &&
                        //                 actors[i]->is<PxRigidDynamic>()->isSleeping();

                            Matrix temp = Matrix(shapePose.front()) *
                                      Matrix::CreateScale(
                                          m_simToRenderScale); // PhysX to Render 스케일
                    
                            Vector3 tempRot, tempPos;
                            Model::ExtractEulerAnglesFromMatrix(&temp, tempRot);
                            Model::ExtractPositionFromMatrix(&temp, tempPos);

                            m_PhysicalObjects[count]->UpdateTranseform(
                                Vector3(m_simToRenderScale), tempRot, tempPos);

                        //m_PhysicalObjects[count]->UpdateWorldRow(temp );
                        //m_PhysicalObjects[count]->UpdateScale(
                        //    Vector3(*shapePose.front() * m_simToRenderScale));

                        m_PhysicalObjects[count]->UpdateConstantBuffers(m_device,
                                                                        m_context);

                        count++;
                    }
                }
            }
    }

    /* PxContactPair 추출 하면 내 캐릭터가 어디에 닿았는지 찾을 수 있음
    * void onContact(const PxContactPairHeader& pairHeader, const PxContactPair*
    pairs, PxU32 nbPairs)
    {
        PX_UNUSED((pairHeader));
        std::vector<PxContactPairPoint> contactPoints;

        for(PxU32 i=0;i<nbPairs;i++)
        {  
            PxU32 contactCount = pairs[i].contactCount;
            if(contactCount)
            {
                contactPoints.resize(contactCount);
                pairs[i].extractContacts(&contactPoints[0], contactCount);

                for(PxU32 j=0;j<contactCount;j++)
                {
                    gContactPositions.push_back(contactPoints[j].position);
                    gContactImpulses.push_back(contactPoints[j].impulse);
                }
            }
        }
    }
    */
}

void Ex2001_GamePlay::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void Ex2001_GamePlay::InitAudio() {

        m_audEngine = std::make_unique<DirectX::AudioEngine>();

        m_sound = std::make_unique<DirectX::SoundEffect>(m_audEngine.get(),
                                                         L"../Assets/Sound/"
                                                         L"Swoosh-4S.wav");
        
}
 



 
void Ex2001_GamePlay::UpdateGUI() {
    AppBase::UpdateGUI();
 //   ImGui::SetNextItemOpen(false, ImGuiCond_Once); 
    Vector3 tempPos = m_camera->GetEyePos();
    float camera[3] = {tempPos.x, tempPos.y, tempPos.z};
    ImGui::DragFloat3("Camera Pos", camera, 1.0f, -1000.f, 1000.f);
    ImGui::DragFloat("DirectionalLightPow",
                     &m_globalConstsCPU.directionalLightPow, 0.01f, 0.0f, 1.0f);
    if (ImGui::TreeNode("Shadow")) 
    {
        static int i = 0;
        ImGui::SliderInt("Shadow index", &i, 0, 4);
         
            ImGui::Text("_______");
            float *temp[2] = 
            {
                &m_shadowAspects[i].x,
                &m_shadowAspects[i].y,  
            }; 
             
            ImGui::DragFloat2("Shadow aspect", *temp, 0.025f, -20.f, 20.f); 
             ImGui::DragFloat("Shadow farZ", &m_shadowAspects[i].z, 0.25f, 
                                 0.0f, 1000.f); 

        
    } 
    if (ImGui::TreeNode("Basic")) {
    
        //            m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        //m_globalConstsCPU.lights[0].position = Vector3(0.0f, 4.5f, -3.0f);
        //m_globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
        //m_globalConstsCPU.lights[0].spotPower = 3.0f;
        //m_globalConstsCPU.lights[0].radius = 0.04f;
            //float


        ImGui::SliderFloat("spotPower", &m_globalConstsCPU.lights[0].spotPower,
                           0.0f, 10.f);
        ImGui::SliderFloat("radius", &m_globalConstsCPU.lights[0].radius,
                           0.0f, 0.3f);


            ImGui::Checkbox("BlendAnimation", &bUseBlendAnimation);

                static float oceanHeight = 0.3f;
            if (ImGui::SliderFloat("OceanHeight", &oceanHeight, -1.0f, 1.0f)) {
                Vector3 position = Vector3(0.0f, oceanHeight, 2.0f);

                m_ocean->UpdatePosition(position);

            }

            if (ImGui::TreeNode("General")) {
                ImGui::Checkbox("Use FPV", &m_camera->m_objectTargetCameraMode);
                ImGui::Checkbox("Wireframe", &m_drawAsWire);
                ImGui::Checkbox("DrawOBB", &m_drawOBB);
                ImGui::Checkbox("DrawBSphere", &m_drawBS);
                if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
                    CreateBuffers();
                }
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Skybox")) {
                ImGui::SliderFloat("Strength", &m_globalConstsCPU.strengthIBL, 0.0f,
                                   0.5f);
                ImGui::RadioButton("Env", &m_globalConstsCPU.textureToDraw, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Specular", &m_globalConstsCPU.textureToDraw, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Irradiance", &m_globalConstsCPU.textureToDraw, 2);
                ImGui::SliderFloat("EnvLodBias", &m_globalConstsCPU.envLodBias, 0.0f,
                                   10.0f);
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Post Effects")) {
                int flag = 0;
                flag += ImGui::RadioButton("Render", &m_postEffectsConstsCPU.mode, 1);
                ImGui::SameLine();
                flag += ImGui::RadioButton("Depth", &m_postEffectsConstsCPU.mode, 2);
                flag += ImGui::SliderFloat(
                    "DepthScale", &m_postEffectsConstsCPU.depthScale, 0.0, 1.0);
                flag += ImGui::SliderFloat("Fog", &m_postEffectsConstsCPU.fogStrength,
                                           0.0, 10.0);

                if (flag)
                    D3D11Utils::UpdateBuffer(m_context, m_postEffectsConstsCPU,
                                             m_postEffectsConstsGPU);

                ImGui::TreePop();
            }
               
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
                    m_postProcess.m_combineFilter.UpdateConstantBuffers(m_context);
                }
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (m_mirror && ImGui::TreeNode("Mirror")) {

                ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
                const float blendColor[4] = {m_mirrorAlpha, m_mirrorAlpha,
                                             m_mirrorAlpha, 1.0f};
                if (m_drawAsWire)
                    Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
                else
                    Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

                ImGui::SliderFloat("Metallic",
                                   &m_mirror->m_materialConsts.GetCpu().metallicFactor,
                                   0.0f, 1.0f);
                ImGui::SliderFloat("Roughness",
                                   &m_mirror->m_materialConsts.GetCpu().roughnessFactor,
                                   0.0f, 1.0f);

                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Light")) {
                // ImGui::SliderFloat3("Position",
                // &m_globalConstsCPU.lights[0].position.x,
                //                     -5.0f, 5.0f);
                ImGui::SliderFloat("Halo Radius",
                                   &m_globalConstsCPU.lights[1].haloRadius, 0.0f, 2.0f);
                ImGui::SliderFloat("Halo Strength",
                                   &m_globalConstsCPU.lights[1].haloStrength, 0.0f,
                                   1.0f);
                ImGui::SliderFloat("Radius", &m_globalConstsCPU.lights[1].radius, 0.0f,
                                   0.5f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    int tempMouseMode = (int)m_mouseMode;
    if (ImGui::SliderInt(
        "MouseMode", &tempMouseMode, 0, 2,
            "0 : None  1 : ObjectPickingMode  2: HeightMapEditMode")) {
            m_mouseMode = (EMouseMode)tempMouseMode;
    }
    if (m_mouseMode == EMouseMode::TextureMapEditMode) {
            int type = (int)m_textureType;

            if (ImGui::SliderInt("EditTexture", &type, -1, 3,
                                 "-1: None")) {
                m_textureType = (EEditTextureType)type; 
            }
            
            ImGui::SliderFloat("EditRadius", &m_groundPlane->editRadius, 1.0f,
                               50.0f);
    }
    if (m_pickedModel && m_pickedModel->m_editable) {
            if (ImGui::TreeNode("Object Editor")) {

                ImGui::Checkbox("ObjectLock", &m_pickedModel->isObjectLock);

                if (ImGui::Checkbox("Render BVH",
                                    &m_pickedModel->bRenderingBVH)) {
                    // m_pickedModel->maxRenderingBVHLevel = 0;
                }
                ImGui::SliderInt("BVH Level",
                                 &m_pickedModel->maxRenderingBVHLevel, 0, 30);

                if (m_keyPressed['Q']) {
                    if (ImGui::ColorButton("Pos",
                                           ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0,
                                           ImVec2(30, 30))) {
                        cout << "Click! Pos Button" << endl;
                    }
                } else if (m_keyPressed['W']) {
                    if (ImGui::ColorButton("Rot",
                                           ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 0,
                                           ImVec2(30, 30))) {
                        cout << "Click! Pos Button" << endl;
                    }
                }

                if (ImGui::TreeNode("Transform")) {

                    float modelLocation[3] = {m_pickedModel->GetPosition().x,
                                              m_pickedModel->GetPosition().y,
                                              m_pickedModel->GetPosition().z};
                    float modelRotation[3] = {m_pickedModel->GetRotation().x,
                                              m_pickedModel->GetRotation().y,
                                              m_pickedModel->GetRotation().z};

                    modelRotation[0] *= 180.f / 3.141592f;
                    modelRotation[1] *= 180.f / 3.141592f;
                    modelRotation[2] *= 180.f / 3.141592f;

                    float modelScale[3] = {m_pickedModel->GetScale().x,
                                           m_pickedModel->GetScale().y,
                                           m_pickedModel->GetScale().z};

                    int flagLotation = ImGui::DragFloat3(
                        "Position", modelLocation, 0.01f, -30.0f, 30.f);
                    int flagRotation = ImGui::DragFloat3(
                        "Rotation", modelRotation, 5.f, -720.0f, 720.f);
                    int flagScale = ImGui::DragFloat3("Scale", modelScale,
                                                      0.01f, -50.0f, 50.f);

                    if (flagLotation) {
                        m_pickedModel->UpdatePosition(
                            Vector3(modelLocation[0], modelLocation[1],
                                    modelLocation[2]));
                    }
                    if (flagRotation) {
                        modelRotation[0] *= 3.141592f / 180.f;
                        modelRotation[1] *= 3.141592f / 180.f;
                        modelRotation[2] *= 3.141592f / 180.f;

                        m_pickedModel->UpdateRotation(
                            Vector3(modelRotation[0], modelRotation[1],
                                    modelRotation[2]));
                    }
                    if (flagScale) {
                        m_pickedModel->UpdateScale(Vector3(
                            modelScale[0], modelScale[1], modelScale[2]));
                    }
                    ImGui::TreePop();
                }
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Material")) {
                    ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias,
                                       0.0f, 10.0f);
                    int flag = 0;

                    if (m_pickedModel) {

                        flag += ImGui::SliderFloat(
                            "minMetallic",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .minMetallic,
                            0.0f, 1.0f);
                        flag += ImGui::SliderFloat(
                            "minRoughness",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .minRoughness,
                            0.0f, 1.0f);

                        flag += ImGui::SliderFloat(
                            "Metallic",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .metallicFactor,
                            0.0f, 1.0f);
                        flag += ImGui::SliderFloat(
                            "Roughness",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .roughnessFactor,
                            0.0f, 1.0f);
                        flag += ImGui::SliderInt(
                            "invertNormalMapY",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .invertNormalMapY,
                            0, 1);

                        flag += ImGui::CheckboxFlags(
                            "AlbedoTexture",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .useAlbedoMap,
                            1);
                        flag += ImGui::CheckboxFlags(
                            "EmissiveTexture",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .useEmissiveMap,
                            1);
                        flag += ImGui::CheckboxFlags(
                            "Use NormalMapping",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .useNormalMap,
                            1);
                        flag += ImGui::CheckboxFlags(
                            "Use AO",
                            &m_pickedModel->m_materialConsts.GetCpu().useAOMap,
                            1);
                        flag += ImGui::CheckboxFlags(
                            "Use HeightMapping",
                            &m_pickedModel->m_meshConsts.GetCpu().useHeightMap,
                            1);
                        flag += ImGui::SliderFloat(
                            "HeightScale",
                            &m_pickedModel->m_meshConsts.GetCpu().heightScale,
                            0.0f, 0.1f);
                        flag += ImGui::CheckboxFlags(
                            "Use MetallicMap",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .useMetallicMap,
                            1);
                        flag += ImGui::CheckboxFlags(
                            "Use RoughnessMap",
                            &m_pickedModel->m_materialConsts.GetCpu()
                                 .useRoughnessMap,
                            1);
                        if (flag) {
                            m_pickedModel->UpdateConstantBuffers(m_device,
                                                                 m_context);
                        }

                        ImGui::Checkbox("Draw Normals",
                                        &m_pickedModel->m_drawNormals);
                    }

                        ImGui::TreePop();
                }
                ImGui::TreePop();
            }
    }
     
        ImGui::NewLine();
         

         
    if (ImGui::TreeNode("Load Object")) {

            bool show_dialog = true;
            ImGui::SetNextWindowSize(ImVec2(400, 500));
            ImGui::SetNextWindowPos(ImVec2(
                ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f));
            ImGui::Begin("Dialogx1", &show_dialog, ImGuiWindowFlags_NoCollapse);

        if (ImGui::TreeNode("Object List")) {
                for (auto object : m_JsonManager->objectInfo) {
                    if (ImGui::Button(object.second.c_str(), ImVec2(300, 30))) {
                        ObjectSaveInfo temp;
                        temp.meshID = (int)object.first;
                        temp.position = RayCasting(0.0f, 0.0f);

                        float dist = 0.0f;
                        SetHeightPosition(temp.position +
                                              Vector3(0.0f, 5.0f, 0.0f),
                                          Vector3(0.0f, -1.0f, 0.0f), dist);
                        if (dist > 0.0f)
                            temp.position = temp.position +
                                            Vector3(0.0f, 5.0f, 0.0f) +
                                            Vector3(0.0f, -1.0f, 0.0f) * dist;

                        shared_ptr<Model> tempModel =
                            m_JsonManager->CreateMesh(temp);

                        Vector3 tempExtents =
                            Vector3::Transform(tempModel->m_boundingBox.Extents,
                                               tempModel->m_worldRow);
                        tempModel->UpdatePosition(
                            tempModel->GetPosition() +
                            Vector3(0.0f, tempExtents.y, 0.0f));
                    }
                }
                ImGui::TreePop();
        }
        if (ImGui::TreeNode("GlTF List")) {

                for (auto object : m_JsonManager->meshPaths) {
                    if (ImGui::Button(object.first.c_str(), ImVec2(300, 30))) {
                             
                        ObjectSaveInfo temp;
                        temp.meshID = -1;
                        temp.meshName = object.first;
                        temp.meshPath = object.second.first;
                        temp.previewPath = object.second.second;
                        temp.position = RayCasting(0.0f, 0.0f);
                        float dist = 0.0f;
                        SetHeightPosition(temp.position +
                                              Vector3(0.0f, 5.0f, 0.0f),
                                          Vector3(0.0f, -1.0f, 0.0f), dist);
                        if (dist > 0.0f)
                            temp.position = temp.position +
                                            Vector3(0.0f, 5.0f, 0.0f) +
                                            Vector3(0.0f, -1.0f, 0.0f) * dist;
                        shared_ptr<Model> tempModel =
                            m_JsonManager->CreateMesh(temp);
                        Vector3 tempExtents =
                            Vector3::Transform(tempModel->m_boundingBox.Extents,
                                               tempModel->m_worldRow);
                        tempModel->UpdatePosition(
                            tempModel->GetPosition() +
                            Vector3(0.0f, tempExtents.y, 0.0f));
                    }
                } 
                ImGui::TreePop();
        }
        if (ImGui::TreeNode("Quicell List")) {

                static vector<int> idV(m_JsonManager->quicellPaths.size());
                int i = 0;
                for (auto object : m_JsonManager->quicellPaths) {
                    // if (ImGui::Button(object.second.mesh.c_str(), ImVec2(300,
                    // 30)))
                    ImGui::NewLine();  
                      ObjectSaveInfo temp;
                    if (object.second.mesh.size() > 1) { 
                        ImGui::SliderInt(object.first.c_str(), &idV[i],
                                         0,
                                         object.second.mesh.size() - 1); 
                        temp.quixelID = idV[i++]; 
                    }    
                    if (ImGui::ImageButton(object.second.objectImageSRV.Get(),
                                           ImVec2(100, 100))) {

                        temp.meshID = -2;
                        temp.quicellPath = object.first;
                        temp.rotation =
                            Vector3(3.141592f * 90.f / 180.f, 0.0f, 0.0f);
                        temp.minMetallic = 0.5f;
                        temp.minRoughness = 1.0f;
                        temp.position = RayCasting(0.0f, 0.0f);

                        float dist = 0.0f;
                        SetHeightPosition(temp.position +
                                              Vector3(0.0f, 5.0f, 0.0f),
                                          Vector3(0.0f, -1.0f, 0.0f), dist);
                        if (dist > 0.0f)
                            temp.position = temp.position +
                                            Vector3(0.0f, 5.0f, 0.0f) +
                                            Vector3(0.0f, -1.0f, 0.0f) * dist;
                           
                        shared_ptr<Model> tempModel =
                            m_JsonManager->CreateMesh(temp);
                         
                        Matrix tempRow = tempModel->m_worldRow;
                        tempRow.Translation(Vector3(0.0f));
                        Vector3 tempExtents = Vector3::Transform(
                            tempModel->m_boundingBox.Extents, tempRow);

                        tempModel->UpdatePosition(tempModel->GetPosition() + Vector3(0.0f, std::abs(tempExtents.y), 0.0f));  

                        UpdateBVH(); 
                    }
                }
                ImGui::TreePop();
        }
            /*if (ImGui::Button("Test", ImVec2(100, 100)))*/
                
          ImGui::TreePop();
        ImGui::End();
    }  
     
   if (ImGui::Button("Update BVH", ImVec2(100, 20))) 
        UpdateBVH(); 
   //ImGui::Checkbox("Render BVH", &m_bRenderingBVH); 
   //ImGui::SliderInt("BVH Level", &maxBVHRenderLevel, 0, 10);  
   //ImGui::SliderFloat("projFovAngleY", &m_camera->m_projFrustumAngleY,
   //                   5.0f, 
   //                   100.f);  
   //ImGui::SliderFloat("projFovAspect", &m_camera->m_projFrustumAspect,
   //                   4.0f / 9.f, 21.f / 9.f);
   //ImGui::SliderFloat("frustumSize", &frustumSize,
   //                   0.0f, 1.0f); 
   ImGui::Checkbox("Character View", &m_camera->m_UsingCharacterView);
   //ImGui::SliderInt("BVNode Box", &m_BVHNodeID, 0, m_BVNodes.size() - 1);
}  

} // namespace hlab
