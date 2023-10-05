#include "JsonManager.h"
#include "AppBase.h"
#include "Model.h"

using namespace std;
using namespace rapidjson;

hlab::JsonManager::JsonManager(AppBase *appBase) {
        m_appBase = appBase; 
        m_saveFile = Document(kObjectType);
}

bool hlab::JsonManager::ParseJson(rapidjson::Document &doc,
                                  const std::string &jsonData) {
    if (doc.Parse(jsonData.c_str()).HasParseError()) {
        return false;
    }

    return doc.IsObject();
}

std::string hlab::JsonManager::JsonDocToString(rapidjson::Document &doc,bool isPretty) {

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
    //Document doc(kObjectType);

    //Document::AllocatorType &allocator = doc.GetAllocator();
    //doc.AddMember("project", "rapidjson", allocator);
    //doc.AddMember("stars", 10, allocator);
    //doc.AddMember("abcd", 0.0, allocator);

    //std::string jsonString = JsonDocToString(doc, true);

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
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    m_saveFile.ParseStream(is);

        for (rapidjson::SizeType i = 0; i < m_saveFile.Size(); ++i) {
                ObjectSaveInfo temp;
                temp.meshID = m_saveFile[i]["meshID"].GetInt();
                temp.meshName = m_saveFile[i]["meshName"].GetString();
                temp.objectID = m_saveFile[i]["objectID"].GetInt();

                auto position = m_saveFile[i]["position"].GetObj();
                temp.position.x = position["x"].GetFloat();
                temp.position.y = position["y"].GetFloat();
                temp.position.z = position["z"].GetFloat();


                auto rotation = m_saveFile[i]["rotation"].GetObj();
                temp.rotation.x = rotation["x"].GetFloat();
                temp.rotation.y = rotation["y"].GetFloat();
                temp.rotation.z = rotation["z"].GetFloat();

                auto scale = m_saveFile[i]["scale"].GetObj();
                temp.scale.x = scale["x"].GetFloat();
                temp.scale.y = scale["y"].GetFloat();
                temp.scale.z = scale["z"].GetFloat();
        
     }
}

void hlab::JsonManager::SaveMesh() 
{
     m_saveFile.Clear();
    Document::AllocatorType &allocator = m_saveFile.GetAllocator();
     m_saveFile.SetArray();

     for (auto object : m_appBase->m_objects) {
                ObjectSaveInfo meshInfo = object->objectInfo;

                rapidjson::Value value;
                // rapidjson::Value value;
                value.AddMember("meshID", Value(meshInfo.meshID), allocator);
                value.AddMember("meshName", Value(*meshInfo.meshName.c_str()),
                                allocator);
                value.AddMember("objectID", Value(meshInfo.objectID),
                                allocator);

                rapidjson::Value scale;
                scale.AddMember("x", Value(meshInfo.scale.x), allocator);
                scale.AddMember("y", Value(meshInfo.scale.y), allocator);
                scale.AddMember("z", Value(meshInfo.scale.z), allocator);
                value.AddMember("scale", scale, allocator);

                rapidjson::Value position;
                position.AddMember("x", Value(meshInfo.position.x), allocator);
                position.AddMember("y", Value(meshInfo.position.y), allocator);
                position.AddMember("z", Value(meshInfo.position.z), allocator);
                value.AddMember("position", position, allocator);

                rapidjson::Value rotation;
                rotation.AddMember("x", Value(meshInfo.rotation.x), allocator);
                rotation.AddMember("y", Value(meshInfo.rotation.y), allocator);
                rotation.AddMember("z", Value(meshInfo.rotation.z), allocator);
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
