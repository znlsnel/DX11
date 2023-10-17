#pragma once
#include <filesystem>
#include "JsonManager.h"
#include "AppBase.h"

#include "BillboardModel.h"

#include "Character.h"


using namespace std;
using namespace rapidjson;
namespace fs = std::filesystem;
namespace hlab {

hlab::JsonManager::JsonManager(AppBase *appBase) {
    m_appBase = appBase;
    m_saveFile = Document(kArrayType);
    LoadObjectPathInFolder();
}

bool hlab::JsonManager::ParseJson(rapidjson::Document &doc,
                                  const std::string &jsonData) {
    if (doc.Parse(jsonData.c_str()).HasParseError()) {
        return false;
    }

    
    return doc.IsObject();
}

std::string hlab::JsonManager::JsonDocToString(rapidjson::Document &doc,
                                               bool isPretty) {

    StringBuffer buffer;
    if (isPretty) {
        PrettyWriter<StringBuffer> writer(buffer);
        doc.Accept(writer);
    } else {
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
    }
    return buffer.GetString();
}

void JsonManager::LoadObjectPathInFolder() { 
        
        const fs::path projectPath =
            fs::current_path().parent_path().parent_path();
        const fs::path tempPath =
            projectPath / "models" / "glTF-Sample-Models-master" / "2.0";
        modelsPath = tempPath.string();

        SearchModelFiles(tempPath);
        
}

void JsonManager::SearchModelFiles(const filesystem::path &directory) {


        for (const auto &entry : fs::directory_iterator(directory)) {

                if (entry.is_directory()) {
                            SearchModelFiles(entry);
        
                } else if (entry.is_regular_file()) {


                        const auto &filePath = entry.path();
                            const auto &fileName = entry.path()
                                                       .parent_path()
                                                       .parent_path()
                                                       .filename();

                        if (filePath.parent_path().filename() == "glTF" 
                                && filePath.extension() == ".gltf") {

                            cout << "##########" << fileName
                                 << "##########" << endl;
                                       
                            std::cout << "Found gltf file : "
                                      << filePath.filename()
                                << endl;
                            meshPaths.insert(make_pair(fileName.string(),
                                          make_pair(filePath.filename().string(), "")));

                        }
                        
                        if (filePath.parent_path().filename() == "screenshot" &&
                            filePath.stem() == "screenshot") {
                        
                                std::cout << "Found screenshot file : "
                                      << filePath.filename() << endl;
                                auto value = meshPaths.find(fileName.string());

                                if (value != meshPaths.end()) 
                                        value->second.second = filePath.filename().string();
                                
                                
                        }
        
                }



                

        }
}


void hlab::JsonManager::LoadMesh() {

    FILE *fp = fopen("saveFile.json", "r");
    if (!fp) {
            // file load failed
        return;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    m_saveFile.ParseStream(is);
    fclose(fp);

    if (m_saveFile.IsArray()) {
            for (rapidjson::SizeType i = 0; i < m_saveFile.Size(); ++i) {
            const rapidjson::Value &object = m_saveFile[i];

                ObjectSaveInfo temp;
                temp.meshID = object["meshID"].GetInt();
                if (temp.meshID == -1) {
                        const rapidjson::Value &path =
                                object["filePath"].GetObj();
                        temp.meshName = path["object"].GetString();
                        temp.meshPath = path["objectPath"].GetString();
                        temp.previewPath =
                                path["screenshotPath"].GetString();
                }
         //       temp.meshName = object["meshName"].GetString();

                const rapidjson::Value & position = object["position"].GetObj();
                temp.position.x = position["x"].GetFloat();
                temp.position.y = position["y"].GetFloat();
                temp.position.z = position["z"].GetFloat();

                const rapidjson::Value &rotation = object["rotation"].GetObj();
                temp.rotation.x = rotation["x"].GetFloat();
                temp.rotation.y = rotation["y"].GetFloat();
                temp.rotation.z = rotation["z"].GetFloat();

                const rapidjson::Value &scale = object["scale"].GetObj();
                temp.scale.x = scale["x"].GetFloat();
                temp.scale.y = scale["y"].GetFloat();
                temp.scale.z = scale["z"].GetFloat();

                CreateMesh(temp);
            }
    }
}

void hlab::JsonManager::SaveMesh() {
    //m_saveFile.Clear();
    m_saveFile = Document(kArrayType);
    Document::AllocatorType &allocator = m_saveFile.GetAllocator();
    //m_saveFile.SetArray();

    for (auto object : m_appBase->m_objects) {

            // TODO ObjectSaveInfo 정보를 새롭게 갱신해서 넣어주기
            ObjectSaveInfo meshInfo;
            meshInfo.meshID = object.second->objectInfo.meshID;
            meshInfo.position = object.second->GetPosition();
            meshInfo.rotation = object.second->GetRotation();
            meshInfo.scale = object.second->GetScale();
            // meshInfo.meshName = "tsetName";

            rapidjson::Value value(kObjectType);
            // rapidjson::Value value;
            value.AddMember("meshID", Value(meshInfo.meshID), allocator);

            const char* name = meshInfo.meshName.c_str();
            const char* meshPath = meshInfo.meshPath.c_str();
            const char* previewPath = meshInfo.previewPath.c_str();
            rapidjson::GenericStringRef nameRef(name);
            rapidjson::GenericStringRef meshPathRef(meshPath);
            rapidjson::GenericStringRef previewPathRef(previewPath);

        rapidjson:Value filePath(kObjectType);
        filePath.AddMember("object", nameRef, allocator);
        filePath.AddMember("objectPath", meshPathRef, allocator);
        filePath.AddMember("screenshotPath", previewPathRef,
                        allocator);

        rapidjson::Value scale(kObjectType);
        scale.AddMember("x", Value(meshInfo.scale.x), allocator);
        scale.AddMember("y", Value(meshInfo.scale.y), allocator);
        scale.AddMember("z", Value(meshInfo.scale.z), allocator);

        rapidjson::Value position(kObjectType);
        position.AddMember("x", Value(meshInfo.position.x), allocator);
        position.AddMember("y", Value(meshInfo.position.y), allocator);
        position.AddMember("z", Value(meshInfo.position.z), allocator);

        rapidjson::Value rotation(kObjectType);
        rotation.AddMember("x", Value(meshInfo.rotation.x), allocator);
        rotation.AddMember("y", Value(meshInfo.rotation.y), allocator);
        rotation.AddMember("z", Value(meshInfo.rotation.z), allocator);

        value.AddMember("filePath", filePath, allocator);
        value.AddMember("scale", scale, allocator);
        value.AddMember("position", position, allocator);
        value.AddMember("rotation", rotation, allocator);

        m_saveFile.PushBack(value, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    m_saveFile.Accept(writer);

    FILE *fp = fopen("saveFile.json", "wb");
    fwrite(buffer.GetString(), sizeof(char), buffer.GetSize(), fp);
    fclose(fp);

    // Debug Dragon
    /*std::string jsonString = JsonDocToString(m_saveFile, true);*/
}

void hlab::JsonManager::CreateMesh(ObjectSaveInfo temp) {
    shared_ptr<Model> tempMesh = make_shared<Model>();

    switch ((meshID)temp.meshID) {
    case meshID::ELoadToPath: {
        tempMesh = CreateModel(temp);
        tempMesh->objectInfo.meshName = temp.meshName;
        tempMesh->objectInfo.meshPath = temp.meshPath;
        tempMesh->objectInfo.previewPath = temp.previewPath;
    } 
                            break;
    case meshID::ECharacter: {
        tempMesh = CreateCharacter(temp);
        tempMesh->objectInfo.meshName = "Character";
    }
        break;
    case meshID::EMountain: {
        tempMesh = CreateMountain(temp);
        tempMesh->objectInfo.meshName = "Mountain";
    }
        break;
    case meshID::ECylinder: {
        tempMesh = CreateCylinder(temp);
        tempMesh->objectInfo.meshName = "Cylinder";

    }
        break;
    case meshID::EPlane: {
        tempMesh = CreatePlane(temp);
        tempMesh->objectInfo.meshName = "Ground";
    }
        break;
    case meshID::ESphere: {
        tempMesh = CreateSphere(temp);
        tempMesh->objectInfo.meshName = "Sphere";
    }
        break;
    case meshID::ESquare: {
        tempMesh = CreateSquare(temp);
        tempMesh->objectInfo.meshName = "Square";
    }
        break;
    case meshID::EBox: {
        tempMesh = CreateBox(temp);
        tempMesh->objectInfo.meshName = "Box";
    }
        break;
    }
    
    static int objectID = 1;
    if (tempMesh != nullptr) {
        tempMesh->objectInfo.objectID = objectID;
        tempMesh->objectInfo.meshID = temp.meshID;
        tempMesh->m_meshConsts.GetCpu().indexColor[0] = 
        (float)objectID / 255;
    //        Vector4(objectID, 0.0f, 0.0f, 1.0f);

        std::cout << "Set [" << tempMesh->objectInfo.meshName
                  << "] Object ID : " << objectID << endl; 

        m_appBase->m_objects.insert(make_pair(objectID, tempMesh));
        objectID++;
    }
}

shared_ptr<class Model> JsonManager::CreateModel(ObjectSaveInfo info) {

    auto meshes = GeometryGenerator::ReadFromFile(
        modelsPath + "\\" + info.meshName + "\\glTF\\", info.meshPath, false, true);

    shared_ptr<Model> tempModel =
        make_shared<Model>(m_appBase->m_device, m_appBase->m_context, meshes);

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

    m_appBase->m_basicList.push_back(tempModel); // 리스트에 등록
    m_appBase->m_pbrList.push_back(tempModel);   // 리스트에 등록

    return tempModel;
}

std::shared_ptr<class Model> JsonManager::CreateCharacter(ObjectSaveInfo info) {
    vector<string> clipNames = {"Idle.fbx", "walk_start.fbx", "walk.fbx",
                                "walk_end.fbx", "fireBall.fbx"};
    string path = "../Assets/Characters/Mixamo/";

    // auto [meshes, _] =
    //      GeometryGenerator::ReadAnimationFromFile(path,
    //      "Character_Kachujin.fbx");

    shared_ptr<Character> m_player = make_shared<Character>(m_appBase, m_appBase->m_device, m_appBase->m_context, path,
                                      "Character_Kachujin.fbx", clipNames);

    //m_player->GetMesh()->UpdateWorldRow(
    //    Matrix::CreateScale(info.scale.x) *
    //    Matrix::CreateRotationX(info.rotation.x) *
    //    Matrix::CreateRotationY(info.rotation.y) *
    //    Matrix::CreateRotationZ(info.rotation.z) *
    //    Matrix::CreateTranslation(info.position)
    //    );
    m_player->GetMesh()->UpdateTranseform(info.scale, info.rotation,
                                          info.position);
   
    m_appBase->m_basicList.push_back(m_player->GetMesh()); // 리스트에 등록
    m_appBase->m_characters.push_back(m_player);
    // m_pickedModel = m_player->GetMesh()->m_meshes;
    m_appBase->m_pbrList.push_back(m_player->GetMesh()); // 리스트에 등록

    m_appBase->m_camera->SetTarget(m_player);
    return m_player->GetMesh();
    //return make_shared
}

shared_ptr<Model> JsonManager::CreateMountain(ObjectSaveInfo info) {

    auto meshes = GeometryGenerator::ReadFromFile("../Assets/Terrain/Chalaadi/",
                                                  "2.fbx", false);

    for (auto &v : meshes[0].vertices)
        v.texcoord /= 1024.0f;
    meshes[0].albedoTextureFilename = "../Assets/Terrain/Chalaadi/overlay.png";

    Vector3 center(0.f, 0.02f, 0.f);
    shared_ptr<Model> tempModel =
        make_shared<Model>(m_appBase->m_device, m_appBase->m_context, meshes);

    tempModel->m_materialConsts.GetCpu().roughnessFactor = 0.97f;
    tempModel->m_materialConsts.GetCpu().metallicFactor = 0.03f;

    // m_terrain->UpdateWorldRow(Matrix::CreateScale(terrainScale) *
    //                           Matrix::CreateTranslation(center));

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;


    m_appBase->m_basicList.push_back(tempModel); // 리스트에 등록
    m_appBase->m_pbrList.push_back(tempModel);   // 리스트에 등록

    return tempModel;
}

shared_ptr<Model> JsonManager::CreateCylinder(ObjectSaveInfo info) {

    auto meshes = 
        GeometryGenerator::MakeCylinder(1.0f, 1.0f, 1.0f, 100.f);
    
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

    m_appBase->m_basicList.push_back(tempModel);
    m_appBase->m_pbrList.push_back(tempModel);
    return tempModel;
}

shared_ptr<Model> JsonManager::CreatePlane(ObjectSaveInfo info) {


    auto meshes = GeometryGenerator::MakeSquare();
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

    m_appBase->m_basicList.push_back(tempModel);
    m_appBase->m_pbrList.push_back(tempModel);
    return tempModel;
}

shared_ptr< Model> JsonManager::CreateSphere(ObjectSaveInfo info) {
    auto meshes = GeometryGenerator::MakeSphere(1.f, 25, 25);
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

    m_appBase->m_basicList.push_back(tempModel);
    m_appBase->m_pbrList.push_back(tempModel);
    return tempModel;
}

shared_ptr<Model> JsonManager::CreateSquare(ObjectSaveInfo info) {

    return make_shared<Model>();
}

shared_ptr<Model> JsonManager::CreateBox(ObjectSaveInfo info) {
    auto meshes = GeometryGenerator::MakeBox(1.0f);
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

    m_appBase->m_basicList.push_back(tempModel);
    m_appBase->m_pbrList.push_back(tempModel);
    return tempModel;
    //return make_shared<Model>();
}





} // namespace hlab