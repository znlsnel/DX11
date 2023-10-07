#include "JsonManager.h"
#include "AppBase.h"
#include "Model.h"
#include "BillboardModel.h"
#include "GeometryGenerator.h"
#include "Character.h"
#include "SkinnedMeshModel.h"


using namespace std;
using namespace rapidjson;
namespace hlab {

hlab::JsonManager::JsonManager(AppBase *appBase) {
    m_appBase = appBase;
    m_saveFile = Document(kArrayType);
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

void hlab::JsonManager::TestJson_Parse() {
    // 1. Parse a JSON string into DOM.
    const char *json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document doc;
    ParseJson(doc, json);
   /* scale.AddMember("z", Value(meshInfo.scale.z), allocator);*/
    Document::AllocatorType &allocator = doc.GetAllocator();
    doc.AddMember("test", "testProject!", allocator);
    
    // 2. Modify it by DOM.
    Value &s = doc["stars"];
    s.SetInt(s.GetInt() + 1);

    std::string jsonString = JsonDocToString(doc, true);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    doc.Accept(writer);

    FILE *fp = fopen("saveFile.json", "wb");
    fwrite(buffer.GetString(), sizeof(char), buffer.GetSize(), fp);
    fclose(fp);

    printf(jsonString.c_str());
}

void hlab::JsonManager::TestJson_AddMember() {

    //        // 1. Parse a JSON string into DOM.
    //// Document doc;
    //// doc.SetObject();
    // Document doc(kObjectType);

    // Document::AllocatorType &allocator = doc.GetAllocator();
    // doc.AddMember("project", "rapidjson", allocator);
    // doc.AddMember("stars", 10, allocator);
    // doc.AddMember("abcd", 0.0, allocator);

    // std::string jsonString = JsonDocToString(doc, true);

    Document doc;

    FILE *fp = fopen("saveFile.json", "r");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    doc.ParseStream(is);
    doc["stars"].SetInt(102);

    std::string jsonString = JsonDocToString(doc, true);
    printf(jsonString.c_str());
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
        meshInfo.meshName = "tsetName";

        rapidjson::Value value(kObjectType);
        // rapidjson::Value value;
        value.AddMember("meshID", Value(meshInfo.meshID), allocator);
        value.AddMember("meshName", Value(*meshInfo.meshName.c_str()),
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
    shared_ptr<Model> tempMesh = nullptr;

    switch ((meshID)temp.meshID) {
    case meshID::ECharacter:
        tempMesh = CreateCharacter(temp);
        break;
    case meshID::EMountain:
        tempMesh = CreateMountain(temp);
        break;
    case meshID::ECylinder:
        tempMesh = CreateCylinder(temp);
        break;
    case meshID::EGround:
        tempMesh = CreateGround(temp);
        break;
    case meshID::ESphere:
        tempMesh = CreateSphere(temp);
        break;
    case meshID::ESquare:
        tempMesh = CreateSquare(temp);
        break;
    case meshID::EBox:
        tempMesh = CreateSquare(temp);
        break;
    }
    
    static float objectID = 0.00001f;
    if (tempMesh != nullptr) {
        tempMesh->objectInfo.objectID = objectID;
        m_appBase->m_objects.insert(make_pair(objectID, tempMesh));
        objectID += 0.00001f;
    }
}

std::shared_ptr<Model> JsonManager::CreateCharacter(ObjectSaveInfo info) {
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

    m_appBase->m_camera->SetTarget(m_player.get());
    return m_player->GetMesh();
}

shared_ptr<class Model> JsonManager::CreateMountain(ObjectSaveInfo info) {
    return shared_ptr<class Model>();
}

shared_ptr<class Model> JsonManager::CreateCylinder(ObjectSaveInfo info) {
    return shared_ptr<class Model>();
}

shared_ptr<class Model> JsonManager::CreateGround(ObjectSaveInfo info) {
    return shared_ptr<class Model>();
}

shared_ptr<class Model> JsonManager::CreateSphere(ObjectSaveInfo info) {
    return shared_ptr<class Model>();
}

shared_ptr<class Model> JsonManager::CreateSquare(ObjectSaveInfo info) {
    return shared_ptr<class Model>();
}





} // namespace hlab