#include "Ex2001_GamePlay.h"

#include "BillboardModel.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "OceanModel.h"
#include "Character.h"
#include "AppBase.h"
#include "JsonManager.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex2001_GamePlay::Ex2001_GamePlay() : AppBase() {}

bool Ex2001_GamePlay::InitScene() {

    AppBase::m_globalConstsCPU.strengthIBL = 0.1f;
    AppBase::m_globalConstsCPU.lodBias = 0.0f;

    AppBase::m_camera->Reset(Vector3(1.60851f, 0.409084f, 0.560064f), -1.65915f,
                            0.0654498f);

    AppBase::InitCubemaps(
        L"../Assets/Textures/Cubemaps/HDRI/Sky/", L"skyEnvHDR.dds",
        L"skySpecularHDR.dds", L"skyDiffuseHDR.dds",
        L"skyBrdf.dds");

    AppBase::InitScene();
     
    // 바닥(거울)
    {
        // https://freepbr.com/materials/stringy-marble-pbr/
        //auto mesh = GeometryGenerator::MakeSquare(10.0, {10.0f, 10.0f});
        auto mesh =  GeometryGenerator::MakeSquareGrid(10, 10);
        string path = "../Assets/Textures/PBR/Ground037_4K-PNG/";
        mesh.albedoTextureFilename = path + "Ground037_4K-PNG_Color.png";
        mesh.aoTextureFilename = path + "Ground037_4K-PNG_AmbientOcclusion.png";
      //  mesh.metallicTextureFilename = path + "";
        mesh.normalTextureFilename = path + "Ground037_4K-PNG_NormalDX.png";
        mesh.roughnessTextureFilename = path + "Ground037_4K-PNG_Roughness.png";
        mesh.heightTextureFilename = path + "Ground037_4K-PNG_Displacement.png";
         
        m_ground = make_shared<Model>(m_device, m_context, vector{mesh});
        m_ground->m_materialConsts.GetCpu().albedoFactor = Vector3(0.2f);
        m_ground->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        m_ground->m_materialConsts.GetCpu().metallicFactor = 0.5f;
        m_ground->m_materialConsts.GetCpu().roughnessFactor = 0.3f;
         
        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        //m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
        //                       Matrix::CreateTranslation(position));
        m_ground->UpdateTranseform(m_ground->GetScale(),
                                   Vector3(3.141592f * 0.5f,
                                           m_ground->GetRotation().y,
                                           m_ground->GetRotation().z),
                                   position);
      // m_ground->useTessellation = true; 
         
        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        // m_mirror = m_ground; // 바닥에 거울처럼 반사 구현
        m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X
      
        m_pbrList.push_back(m_ground);   // 리스트에 등록
    }
    
        // terrain
    {
        auto meshes = GeometryGenerator::ReadFromFile(
            "../Assets/Terrain/Chalaadi/",
            "2.fbx", false);

        // meshes[0].albedoTextureFilename =
        //     "../Assets/Terrain/snowy_mountain_with_slopes/"
        //     "Texture.png";

         //auto meshes = GeometryGenerator::ReadFromFile(
         //    "../Assets/Terrain/Chalaadi/", "2.fbx", false);
         for (auto &v : meshes[0].vertices)
             v.texcoord /= 1024.0f;
         meshes[0].albedoTextureFilename =
             "../Assets/Terrain/Chalaadi/overlay.png";

         float terrainScale = 100.f;

        Vector3 center(0.f, 0.02f, 0.f);
        m_terrain =
            make_shared<Model>(m_device, m_context, meshes);
        //m_terrain->m_materialConsts.GetCpu().invertNormalMapY =
        //    true; // GLTF는 true로
        m_terrain->m_materialConsts.GetCpu().roughnessFactor = 0.97f;
        m_terrain->m_materialConsts.GetCpu().metallicFactor = 0.03f;
        //m_terrain->UpdateWorldRow(Matrix::CreateScale(terrainScale) *
        //                          Matrix::CreateTranslation(center));
        m_terrain->UpdateTranseform(Vector3(terrainScale),
                                    m_terrain->GetRotation(), center);
        m_terrain->m_castShadow = true;
        //m_pickedModel = m_terrain;

        m_basicList.push_back(m_terrain); // 리스트에 등록
        m_pbrList.push_back(m_terrain);   // 리스트에 등록
    }


    // Main Object
    {
        //ObjectSaveInfo temp;
        //temp.meshID = (int)meshID::ECharacter;
        //temp.scale = Vector3(0.2f);
        //temp.position = Vector3(0.0f, 0.1f, 0.0f);
        //m_JsonManager->CreateMesh(temp);
      //  vector<string> clipNames = {"Idle.fbx", "walk_start.fbx", "walk.fbx",  "walk_end.fbx", "fireBall.fbx"};
      //  string path = "../Assets/Characters/Mixamo/";

      // // auto [meshes, _] =
      ////      GeometryGenerator::ReadAnimationFromFile(path, "Character_Kachujin.fbx");

      //  m_player = make_shared<Character>(this, m_device, m_context, path,
      //          "Character_Kachujin.fbx", clipNames );

      //  m_basicList.push_back(m_player->GetMesh()); // 리스트에 등록
      //  m_characters.push_back(m_player);
      //  //m_pickedModel = m_player->GetMesh()->m_meshes;
      //  m_pbrList.push_back(m_player->GetMesh()); // 리스트에 등록

      // 
      //  m_camera->SetTarget(m_player.get());
    }

    InitPhysics(true);

    InitAudio();

    // ocean
        {
        auto mesh = GeometryGenerator::MakeSquare(20.0, {10.0f, 10.0f});
        m_ocean =
            make_shared<OceanModel>(m_device, m_context, vector{mesh});
        m_ocean->m_castShadow = false;

        Vector3 position = Vector3(0.0f, 0.1f, 2.0f);
        //m_ocean->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
        //                        Matrix::CreateTranslation(position));
        m_ocean->UpdateTranseform(m_ocean->GetScale(),
                                  Vector3(3.141592f * 0.5f,
                                          m_ocean->GetRotation().y,
                                          m_ocean->GetRotation().z),
                                  position);
        m_basicList.push_back(m_ocean);
    }


    return true;
}

void Ex2001_GamePlay::CreateStack(const PxTransform &t, int numStacks,
                                  int numWidth, PxReal halfExtent) {

    vector<MeshData> box = {GeometryGenerator::MakeBox(halfExtent)};

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
            AppBase::m_basicList.push_back(m_newObj);
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

    m_fireball = std::make_shared<BillboardModel>();
    // m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
    //                        1.0f, L"GameExplosionPS.hlsl");
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
    MousePicking();


    UpdateAnim(dt);

    // 이하 물리엔진 관련

    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);

    // gScene->getActors()
    // PxGeometryType::eBOX: , case PxGeometryType::eSPHERE:

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

                    
                //m_PhysicalObjects[count]->UpdateWorldRow(
                //    Matrix(shapePose.front()) *
                //    Matrix::CreateScale(
                //        m_simToRenderScale) // PhysX to Render 스케일
                //);
                m_PhysicalObjects[count]->UpdateScale(
                    Vector3(*shapePose.front() * m_simToRenderScale));

                m_PhysicalObjects[count]->UpdateConstantBuffers(m_device,
                                                                m_context);

                count++;
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

void Ex2001_GamePlay::UpdateAnim(float dt) {

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

void Ex2001_GamePlay::MousePicking() {

        if (m_leftButton == false)
                return;

        m_leftButton = false;

        Vector3 cursorNdcNear = Vector3(m_mouseNdcX, m_mouseNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_mouseNdcX, m_mouseNdcY, 1.0f);

        Matrix inverseProjView =
            (m_camera->GetViewRow() * m_camera->GetProjRow()).Invert();

        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, inverseProjView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, inverseProjView);
        Vector3 dir = cursorWorldFar - cursorWorldNear;
        dir.Normalize();
        


        for (auto object : m_objects) {
                shared_ptr<Model> temp = object.second;
                SimpleMath::Ray currRay = SimpleMath::Ray(cursorWorldNear, dir);
                float dist = 0.0f;
               
                bool selected = currRay.Intersects(temp->m_boundingBox, dist);
                if (selected) {
                        std::cout << " selected!!!  !!  "  << std::endl;
                        m_pickedModel = temp;
                        break;
                }
        }

}



void Ex2001_GamePlay::UpdateGUI() {
    AppBase::UpdateGUI();
 //   ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    ImGui::Checkbox("BlendAnimation", &bUseBlendAnimation);

        static float oceanHeight = 0.0f;
    if (ImGui::SliderFloat("OceanHeight", &oceanHeight, -1.0f, 1.0f)) {
        Vector3 position = Vector3(0.0f, oceanHeight, 2.0f);
        //m_ocean->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
        //                        Matrix::CreateTranslation(position));
        m_ocean->UpdateTranseform(m_ocean->GetScale(),
                                  Vector3(3.141592f * 0.5f,
                                          m_ocean->GetRotation().y,
                                          m_ocean->GetRotation().z),
                                  position);

    }

    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_camera->m_useFirstPersonView);
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

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material")) {
        ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias, 0.0f, 10.0f);

        int flag = 0;

        if (m_pickedModel) {
            flag += ImGui::SliderFloat(
                "Metallic",
                &m_pickedModel->m_materialConsts.GetCpu().metallicFactor, 0.0f,
                1.0f);
            flag += ImGui::SliderFloat(
                "Roughness",
                &m_pickedModel->m_materialConsts.GetCpu().roughnessFactor, 0.0f,
                1.0f);
            flag += ImGui::CheckboxFlags(
                "AlbedoTexture",
                &m_pickedModel->m_materialConsts.GetCpu().useAlbedoMap, 1);
            flag += ImGui::CheckboxFlags(
                "EmissiveTexture",
                &m_pickedModel->m_materialConsts.GetCpu().useEmissiveMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use NormalMapping",
                &m_pickedModel->m_materialConsts.GetCpu().useNormalMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use AO", &m_pickedModel->m_materialConsts.GetCpu().useAOMap,
                1);
            flag += ImGui::CheckboxFlags(
                "Use HeightMapping",
                &m_pickedModel->m_meshConsts.GetCpu().useHeightMap, 1);
            flag += ImGui::SliderFloat(
                "HeightScale",
                &m_pickedModel->m_meshConsts.GetCpu().heightScale, 0.0f, 0.1f);
            flag += ImGui::CheckboxFlags(
                "Use MetallicMap",
                &m_pickedModel->m_materialConsts.GetCpu().useMetallicMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use RoughnessMap",
                &m_pickedModel->m_materialConsts.GetCpu().useRoughnessMap, 1);
            if (flag) {
                m_pickedModel->UpdateConstantBuffers(m_device, m_context);
            }
            ImGui::Checkbox("Draw Normals", &m_pickedModel->m_drawNormals);
        }

        ImGui::TreePop();
    }
}

} // namespace hlab
