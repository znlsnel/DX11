#pragma once
#include <filesystem>
#include "JsonManager.h"
#include "AppBase.h"

#include "FoliageModel.h"
#include "BillboardModel.h"
#include "D3D11Utils.h"
#include "Character.h"
#include <DirectXMath.h>


using namespace std;
using namespace rapidjson;

namespace fs = std::filesystem;
namespace hlab {
        using namespace DirectX;

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

        fs::path quicellPathTemp =
            fs::current_path().parent_path().parent_path();
        quicellPathTemp = quicellPathTemp / "Assets" / "Quicells";
        quicellPath = quicellPathTemp.string();

        SearchQuicellModels(quicellPathTemp);
      //  C:\Users\dudth\Documents\Megascans Library\Downloaded\UAssets
        int a = 5;
}

vector<int> makeKMPTable(string Pattern) {
        vector<int> table(Pattern.size(), 0);

        for (int tail = 1, head = 0; tail < Pattern.length(); tail++) {
        while (head > 0 && Pattern[tail] != Pattern[head])
            head = table[head - 1];

        if (Pattern[tail] == Pattern[head])
            table[tail] = ++head;
        }

        return table;
}

bool KMP(string str, string pattern) {
        vector<int> table = makeKMPTable(pattern);
        int parentSize = str.size();      // abcaabcabb [ 10 ]
        int patternSize = pattern.size(); // abca [ 4 ]

        int j = 0;
        for (int i = 0; i < parentSize; i++) {
                while (j > 0 && str[i] != pattern[j]) {
                    j = table[j - 1];
                }
                if (str[i] == pattern[j]) {
                    if (j == patternSize - 1) {
                        //cout << i - patternSize + 2 << "에서 패턴을 찾았습니다."
                        //     << endl;
                        j = table[j];
                        return true;
                    } else {
                        j++;
                    }
                }
        }
         
        return false;

}


void JsonManager::SearchQuicellModels(const filesystem::path &directory, int count) {

        if (count > 2)
                return;

        for (const auto &entry : fs::directory_iterator(directory)) {
                 
                if (entry.is_directory())
                        SearchQuicellModels(entry, count + 1);
                else if (entry.is_regular_file()) {
                
                        const auto &filePath = entry.path().parent_path();
                        const auto &fileName = entry.path().filename();

                        if (entry.path().extension() == ".FBX") 
                        {
                            auto it = quicellPaths.find(filePath.string() + "\\");

                            QuicellMeshPathInfo *temp = new QuicellMeshPathInfo();
                            if (it != quicellPaths.end())
                                temp = &it->second;

                            temp->mesh.push_back(fileName.string());

                            if (it == quicellPaths.end()) 
                            {
                                quicellPaths.insert(
                                    make_pair(filePath.string() + "\\", *temp));
                            }
                             
                        }
                        
                        else if (entry.path().extension() == ".HDR") {
                                
                                auto it =
                                    quicellPaths.find(filePath.string() + "\\");
                                QuicellMeshPathInfo *temp = new QuicellMeshPathInfo(); 
                                                           
                                if (it != quicellPaths.end())
                                    temp = &it->second;

                                int formatIndex =
                                    fileName.string().size() - (string(".HDR").size() + 1);

                                // DpRF , ORDp, DpR, DpRA
                                char format = fileName.string()[formatIndex];
                                 
                                bool isBillboardTexture =
                                    KMP(fileName.string(), "Billboard") ||
                                    KMP(fileName.string(), "billboard");
                                    
                                if (isBillboardTexture)
                                    temp->isFolige = true;
                                   
                                if (format == 'D') {  
                                    if (isBillboardTexture)
                                        temp->billboardDiffuse =
                                            fileName.string();
                                    else
                                        temp->Diffuse = fileName.string();
                                } 
                                else if (format == 'N' ) { 
                                    if (isBillboardTexture)
                                        temp->billboardNormal =  
                                            fileName.string();
                                    else
                                        temp->Normal = fileName.string();
                                } else if (format == 'T') {
                                    if (isBillboardTexture)
                                        temp->billboardArt =
                                            fileName.string();
                                    else {
                                        temp->art = fileName.string();
                                    
                                    } 
                                    cout << "Found ART Texture"
                                         << fileName.string() << "\n";
                                } 
                                 
                                  
                                else if (format == 'F') {
                                    temp->Displacement = fileName.string();
                                    temp->Roughness = fileName.string();
                                    temp->metallic = fileName.string();
                                } else if (format == 'p') {
                                    temp->Occlusion = fileName.string();
                                    temp->Roughness = fileName.string();
                                    temp->Displacement = fileName.string();
                                } else if (format == 'R') {
                                    temp->Displacement = fileName.string();
                                    temp->Roughness = fileName.string();
                                } else if (format == 'A') {
                                    temp->Displacement = fileName.string();
                                    temp->Roughness = fileName.string();
                                    temp->Displacement = fileName.string();
                                }
                                  
                                if (it == quicellPaths.end()) {
                                    quicellPaths.insert(make_pair(
                                        filePath.string() + "\\", *temp));
                                }

                        } 
                        else if (entry.path().extension() == ".jpg" ||
                                   entry.path().extension() == ".png") 
                        {
                                 
                                auto it =
                                    quicellPaths.find(filePath.string() + "\\");
                                QuicellMeshPathInfo *temp =
                                    new QuicellMeshPathInfo();

                                if (it != quicellPaths.end()) {
                                            temp = &it->second;
                                }

                                        D3D11Utils::CreateTexture(
                                            m_appBase->m_device, m_appBase->m_context,
                                            entry.path().string(), false,
                                            temp->objectImage, temp->objectImageSRV);

                                if (it == quicellPaths.end()) {
                                            quicellPaths.insert(make_pair(
                                                filePath.string() + "\\",
                                                *temp));        
                                } 
                        }
                }
                 
        }
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

                            meshPaths.insert(make_pair(fileName.string(),
                                          make_pair(filePath.filename().string(), "")));

                        }
                        
                        if (filePath.parent_path().filename() == "screenshot" &&
                            filePath.stem() == "screenshot") {
                        
                                //std::cout << "Found screenshot file : "
                                //      << filePath.filename() << endl;
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
                if (temp.meshID <= -2 ) {
                        const rapidjson::Value &path =
                            object["filePath"].GetObj();
                        temp.quicellPath = path["quicellPath"].GetString();
                        temp.quixelID = path["quixelID"].GetInt();
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

                const rapidjson::Value &material = object["material"].GetObj();
                if (material.IsNull() == false) {
                        temp.metallic = material["metallic"].GetFloat();
                        temp.roughness = material["roughness"].GetFloat();
                        temp.minMetallic = material["minMetallic"].GetFloat();
                        temp.minRoughness = material["minRoughness"].GetFloat();
                }

                const rapidjson::Value &foliage = object["foliage"].GetObj();
                if (foliage.IsNull() == false)
                {  
                        temp.isFolige = true;
                        temp.foliageRange = foliage["CreateRange"].GetFloat();
                        temp.foliageDensity =
                            foliage["CreateDensity"].GetFloat();
                } 
                 
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
            if (object.second->isDestory || object.second->m_saveable == false)
                continue;
            // TODO ObjectSaveInfo 정보를 새롭게 갱신해서 넣어주기
            ObjectSaveInfo meshInfo;
            meshInfo = object.second->objectInfo;
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
            const char *quicellPath = meshInfo.quicellPath.c_str();

        rapidjson:Value filePath(kObjectType); 
         //       cout << "meshName : "  << name << endl;

                //rapidjson::CharType
                filePath.AddMember("object",
                                   rapidjson::Value().SetString(name, allocator), 
                                allocator) ;
                filePath.AddMember(
                    "objectPath",
                    rapidjson::Value().SetString(meshPath, allocator),
                                allocator);
                filePath.AddMember("screenshotPath",
                    rapidjson::Value().SetString(previewPath, allocator),
                                allocator);
                filePath.AddMember(
                    "quicellPath",
                    rapidjson::Value().SetString(quicellPath, allocator),
                    allocator);
                filePath.AddMember(
                    "quixelID", Value(meshInfo.quixelID), allocator);

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

        rapidjson::Value material(kObjectType);
        material.AddMember("metallic", Value(object.second->m_materialConsts.GetCpu().metallicFactor), allocator);
        material.AddMember("roughness",
                           Value(object.second->m_materialConsts.GetCpu().roughnessFactor),
                           allocator);
        material.AddMember("minMetallic",
                           Value(object.second->m_materialConsts.GetCpu().minMetallic),
                           allocator);
        material.AddMember("minRoughness",
                           Value(object.second->m_materialConsts.GetCpu().minRoughness), 
                           allocator);
          
        rapidjson::Value foliage(kObjectType);
        foliage.AddMember("isFoliage",
                          Value(object.second->objectInfo.isFolige), allocator);
        foliage.AddMember("CreateRange",
                          Value(object.second->objectInfo.foliageRange), allocator);
        foliage.AddMember("CreateDensity",
                          Value(object.second->objectInfo.foliageDensity), allocator);
         

        value.AddMember("filePath", filePath, allocator);
        value.AddMember("scale", scale, allocator);
        value.AddMember("position", position, allocator);
        value.AddMember("rotation", rotation, allocator);
        value.AddMember("material", material, allocator);
        if (object.second->objectInfo.isFolige)
                value.AddMember("foliage", foliage, allocator);



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

shared_ptr<Model> hlab::JsonManager::CreateMesh(ObjectSaveInfo temp) {
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
                tempMesh->objectInfo.meshName = "Plane";
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
            case meshID::EBox: 
            {
                tempMesh = CreateBox(temp);
                tempMesh->objectInfo.meshName = "Box";
            }
                break;
            case meshID::EQuicellPath:  
            {
                tempMesh = CreateQuicellModel(temp);
                auto it = quicellPaths.find(temp.quicellPath);
                if (it != quicellPaths.end()) {
                        tempMesh->objectInfo.meshName =   it->second.mesh[temp.quixelID];  
                        tempMesh->objectInfo.quicellPath = it->first;
                }
            } 
                break; 
            case meshID::EQuicellFoliage: {
                tempMesh = CreateQuicellFoliageModel(temp);
                auto it = quicellPaths.find(temp.quicellPath);
                if (it != quicellPaths.end()) {
                        tempMesh->objectInfo.meshName =
                            it->second.mesh[temp.quixelID];
                        tempMesh->objectInfo.quicellPath = it->first; 
                        tempMesh->objectInfo.isFolige = true;
                        tempMesh->objectInfo.foliageDensity = temp.foliageDensity;
                        tempMesh->objectInfo.foliageRange = temp.foliageRange;


                }
            } break;
            case meshID::ETree:
            {
                tempMesh = CreateTree(temp);
                tempMesh->objectInfo.meshName = "Tree";
            }
                break;
            case meshID::EBillboardTree:
            {
                tempMesh = CreateBillboadTree(temp);
                tempMesh->objectInfo.meshName = "billboardTree";
            } 
                break; 
    }
    
    // 256 = 1 
    // 512 = 2 
    if (tempMesh != nullptr) { 
               
            if ((meshID)temp.meshID == meshID::ECharacter)
                m_appBase->AddBasicList(tempMesh, false, true, true);
            else
                 m_appBase->AddBasicList(tempMesh, true, true, true);



        tempMesh->objectInfo.meshID = temp.meshID;
        tempMesh->objectInfo.metallic = temp.metallic;
        tempMesh->objectInfo.roughness = temp.roughness;
        tempMesh->objectInfo.minMetallic = temp.minMetallic;
        tempMesh->objectInfo.minRoughness = temp.minRoughness; 

        tempMesh->m_materialConsts.GetCpu().metallicFactor = temp.metallic;
        tempMesh->m_materialConsts.GetCpu().roughnessFactor = temp.roughness;
        tempMesh->m_materialConsts.GetCpu().minMetallic = temp.minMetallic;
        tempMesh->m_materialConsts.GetCpu().minRoughness = temp.minRoughness;
        tempMesh->UpdateConstantBuffers(m_appBase->m_device, m_appBase->m_context);
         
    } 
     
    return tempMesh; 
} 

shared_ptr<class Model> JsonManager::CreateModel(ObjectSaveInfo info) {

    auto meshes = GeometryGenerator::ReadFromFile(
        modelsPath + "\\" + info.meshName + "\\glTF\\", info.meshPath, false, true);

    shared_ptr<Model> tempModel =
        make_shared<Model>(m_appBase->m_device, m_appBase->m_context, meshes);

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;



    return tempModel;
}
 
shared_ptr<class Model> 
JsonManager::CreateQuicellModel(ObjectSaveInfo info) {
         
    QuicellMeshPathInfo *temp = &quicellPaths.find(info.quicellPath)->second;
    vector<MeshData> *meshes = new vector<MeshData>();

     
    if (temp->hasMeshs.size() > 0 && temp->hasMeshs[info.quixelID]) {
           meshes = &temp->meshs[info.quixelID];
    } 
    else { 
           *meshes = GeometryGenerator::ReadFromFile(info.quicellPath + "\\",
                                                    temp->mesh[info.quixelID],
                                                    false, false);

           if (temp->hasMeshs.size() == 0) {
                 temp->meshs.resize(temp->mesh.size());
                 temp->hasMeshs.resize(temp->mesh.size());
           }
           temp->meshs[info.quixelID] = *meshes;
           temp->hasMeshs[info.quixelID] = true; 
    }
    
            (*meshes)[0].albedoTextureFilename =
               temp->Diffuse == "" ? "" : info.quicellPath + temp->Diffuse;

           (*meshes)[0].normalTextureFilename =
               temp->Normal == "" ? "" : info.quicellPath + temp->Normal;

           (*meshes)[0].heightTextureFilename =
               temp->Displacement == "" ? ""
                                        : info.quicellPath + temp->Displacement;
             
           (*meshes)[0].aoTextureFilename =
               temp->Occlusion == "" ? "" : info.quicellPath + temp->Occlusion;

           (*meshes)[0].roughnessTextureFilename =
               temp->Roughness == "" ? "" : info.quicellPath + temp->Roughness;

           (*meshes)[0].metallicTextureFilename =
               temp->metallic == "" ? "" : info.quicellPath + temp->metallic;

    shared_ptr<Model> tempModel = make_shared<Model>(
               m_appBase->m_device, m_appBase->m_context, *meshes, m_appBase);
    tempModel->objectInfo.quixelID = info.quixelID;
         
    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;
    tempModel->m_drawBackFace = true; 
    tempModel->m_materialConsts.GetCpu().invertNormalMapY = true;
    tempModel->m_materialConsts.GetCpu().useMetallicMap = true;
     
    return tempModel;

}

shared_ptr<class Model>
JsonManager::CreateQuicellFoliageModel(ObjectSaveInfo info) {
    QuicellMeshPathInfo *temp = &quicellPaths.find(info.quicellPath)->second;
    vector<MeshData> *meshes = new vector<MeshData>();
    vector<int> mesheStartID;
    info.scale = Vector3(0.4f, 0.4f, 0.4f); 
    // density = 0 ~ 1 
    float range =   info.foliageRange;
    float density = 1.01f - info.foliageDensity;
    
    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> randId(0.f, float(temp->mesh.size()) - 1.f);
    std::uniform_real_distribution<float> randRange(-range, range);
        
    int CreateNum = max(int(range / density), 1);
    //CreateNum = 1;
      
    while (CreateNum--) { 
           int quixelNum = int(round(randId(gen))); 
           quixelNum = std::clamp(quixelNum, 0, int(temp->mesh.size()) - 1);
    //       cout << "quixelNum : "<< quixelNum << "\n"; 
            //    cout << "meshes Size : " << CreateNum << "\n";
                    
            Vector2 pos; 
            Vector3 GenPos = Vector3(pos.x, 0.0f, pos.y);
            {  
                    do { 
                        const float posX = randRange(gen);
                        const float posY= randRange(gen);
                         
                        pos = Vector2(posX, posY);

                    } while (pos.Length() > range); 
       
                    float dist = 0.0f;
                    m_appBase->SetHeightPosition(
                        info.position + Vector3(pos.x, 3.0f, pos.y),
                                                 Vector3(0.0f, -1.0f, 0.0f), dist);
                    Vector3 resultPos =
                        info.position + Vector3(pos.x, 3.0f - dist, pos.y);


                    Vector3 dir = resultPos - info.position;
                      
                    if (dist > 0.f)  
                        GenPos = Vector3(dir.x, dir.z, -dir.y);   
                    else
                        GenPos = Vector3(pos.x,pos.y , 0.0f);    
                    GenPos *= 1.0f / info.scale.x;
           //         cout << "GenPos.Y : " << GenPos.z << "\n";
            } 

            if (temp->hasMeshs.size() > 0 && temp->hasMeshs[quixelNum]) {
                   vector<MeshData> &tempMD = temp->meshs[quixelNum];
                    for (int id = 0; id < tempMD.size(); id++) {
                        for (auto &vtx : tempMD[id].vertices) { 
                                 vtx.position =  (vtx.position - temp->yOffset[quixelNum]) + GenPos; 
                        }   
                    }     
                    temp->yOffset[quixelNum] = GenPos;

                   mesheStartID.push_back(meshes->size());
                   meshes->insert(meshes->end(), tempMD.begin(), tempMD.end());
            }  
            else { 
                   vector<MeshData> tempMD = GeometryGenerator::ReadFromFile(
                       info.quicellPath + "\\", temp->mesh[quixelNum], false, false);

                        Vector3 minCorner(1000.f); 
                        Vector3 maxCorner(-1000.f);
                        for (int id = 0; id < tempMD.size(); id++) {
                                 for (auto &vtx : tempMD[id].vertices) {
                                        vtx.position += GenPos;           
                                        if (id == 0) {
                                                minCorner = Vector3::Min(minCorner,
                                                                                vtx.position);
                                                maxCorner = Vector3::Max(maxCorner,
                                                                                vtx.position);
                                        }  
                                 }
                        }     
                        float tempOffset = -(maxCorner.z - minCorner.z) / 2; 
                         //tempOffset = 0;
                         
                        for (int id = 0; id < tempMD.size(); id++) {
                                 for (auto &vtx : tempMD[id].vertices) {
                                        vtx.position += Vector3(0.0f, 0.0f, tempOffset);
                                 }
                        }   
                         
                         
                   mesheStartID.push_back(meshes->size());
                   meshes->insert(meshes->end(), tempMD.begin(), tempMD.end());
            
                   if (temp->hasMeshs.size() == 0) {
                         temp->meshs.resize(temp->mesh.size());
                         temp->hasMeshs.resize(temp->mesh.size());
                         temp->yOffset.resize(temp->mesh.size());
                   }  
                   temp->meshs[quixelNum] = tempMD;
                   temp->hasMeshs[quixelNum] = true;
                   temp->yOffset[quixelNum] = GenPos;
            } 
    }  
        
    for (int i : mesheStartID) { 
                (*meshes)[i].albedoTextureFilename =
                temp->Diffuse == "" ? "" : info.quicellPath + temp->Diffuse;

                (*meshes)[i].normalTextureFilename =
                temp->Normal == "" ? "" : info.quicellPath + temp->Normal;

                (*meshes)[i].heightTextureFilename =
                temp->Displacement == "" ? "" : info.quicellPath + temp->Displacement;

                (*meshes)[i].aoTextureFilename = 
                temp->Occlusion == "" ? "" : info.quicellPath + temp->Occlusion;
                
                (*meshes)[i].roughnessTextureFilename =
                temp->Roughness == "" ? "" : info.quicellPath + temp->Roughness;
                  
                (*meshes)[i].metallicTextureFilename =
                temp->metallic == "" ? "" : info.quicellPath + temp->metallic;    

                (*meshes)[i].artTextureFilename =
                    temp->art == "" ? "" : info.quicellPath + temp->art;    
                  
                (*meshes)[i].billboardARTTextureFilename = 
                    temp->billboardArt == "" ? "" : info.quicellPath + temp->billboardArt;    

                (*meshes)[i].billboardDiffuseTextureFilename =
                    temp->billboardDiffuse == "" ? ""  : info.quicellPath + temp->billboardDiffuse;    
                 
                (*meshes)[i].billboardNormalTextureFilename =
                    temp->billboardNormal == "" ? "" : info.quicellPath + temp->billboardNormal;    
    }  
       
    shared_ptr<FoliageModel> tempModel =
        make_shared<FoliageModel>(m_appBase->m_device, m_appBase->m_context, *meshes, m_appBase, mesheStartID);
    tempModel->objectInfo.quixelID = info.quixelID;

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;
    tempModel->m_drawBackFace = true;
    tempModel->m_materialConsts.GetCpu().invertNormalMapY = true;
    tempModel->m_materialConsts.GetCpu().useMetallicMap = true;

    return tempModel;
}

std::shared_ptr<class Model> JsonManager::CreateCharacter(ObjectSaveInfo info) {
    vector<string> clipNames = {
            "ch08_Breathing Idle.fbx", 
            "ch80_Walk.fbx",
            "ch80_Walking Backwards.fbx", 
            "ch80_Standard Run.fbx", 
            "ch80_Running Backward.fbx", 
            "ch80_Jumping_Up.fbx",
            "ch80_Jumping_Down.fbx",
            "ch80_Falling Idle.fbx", 
            "ch80_Left Turn.fbx",
            "ch80_Right Turn.fbx",
    };
     
    string path = "../Assets/Characters/Mixamo/";
      
    shared_ptr<Character> m_player = make_shared<Character>(m_appBase, m_appBase->m_device, m_appBase->m_context, path,
                                      "Ch08_nonPBR.fbx", clipNames);
    m_player->GetMesh()->m_appBase = m_appBase;

    m_appBase->m_characters.push_back(m_player);
      
    m_player->GetMesh()->m_materialConsts.GetCpu().invertNormalMapY = true;
    m_player->GetMesh()->m_materialConsts.GetCpu().roughnessFactor = 1.0f;
    m_player->GetMesh()->m_materialConsts.GetCpu().metallicFactor = 1.0f;
    //m_player->GetMesh()->m_materialConsts.GetCpu().useAOMap = true;
    m_player->GetMesh()->UpdateTranseform(info.scale, info.rotation,
                                          info.position);
   
    // m_pickedModel = m_player->GetMesh()->m_meshes;
     
    m_appBase->m_camera->SetTarget(m_player);
    return m_player->GetMesh();
    //return make_shared
}

shared_ptr<Model> JsonManager::CreateMountain(ObjectSaveInfo info) {

    auto meshes = GeometryGenerator::ReadFromFile("../Assets/Terrain/Chalaadi/",
                                                  "2.fbx", false);

    for (auto &v : meshes[0].vertices)
        v.texcoord /= 1024.0f;
    meshes[0].albedoTextureFilename = 
        "../Assets/Terrain/Chalaadi/overlay.png";

    Vector3 center(0.f, 0.02f, 0.f);
    shared_ptr<Model> tempModel =
        make_shared<Model>(m_appBase->m_device, m_appBase->m_context, meshes);

    tempModel->m_materialConsts.GetCpu().roughnessFactor = 0.97f;
    tempModel->m_materialConsts.GetCpu().metallicFactor = 0.03f;

    // m_terrain->UpdateWorldRow(Matrix::CreateScale(terrainScale) *
    //                           Matrix::CreateTranslation(center));

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;



    return tempModel;
}

shared_ptr<Model> JsonManager::CreateCylinder(ObjectSaveInfo info) {

    auto meshes = 
        GeometryGenerator::MakeCylinder(1.0f, 1.0f, 1.0f, 100.f);
    
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;


    return tempModel;
}

shared_ptr<Model> JsonManager::CreatePlane(ObjectSaveInfo info) {

        //auto mesh = GeometryGenerator::MakeSquareGrid(10, 10, 100.f,
        //                                              Vector2(100.0f, 100.0f));
        //string path = "../Assets/Textures/PBR/Ground037_4K-PNG/";
        //mesh.albedoTextureFilename = path + "Ground037_4K-PNG_Color.png";
        //mesh.aoTextureFilename = path + "Ground037_4K-PNG_AmbientOcclusion.png";
        //mesh.normalTextureFilename = path + "Ground037_4K-PNG_NormalDX.png";
        ////mesh.roughnessTextureFilename = path + "Ground037_4K-PNG_Roughness.png";
        //mesh.heightTextureFilename = path + "Ground037_4K-PNG_Displacement.png";

        //m_appBase->m_ground = make_shared<Model>(
        //    m_appBase->m_device, m_appBase->m_context, vector{mesh});
        //m_appBase->m_ground->m_materialConsts.GetCpu().albedoFactor =
        //    Vector3(0.2f);
        //m_appBase->m_ground->m_materialConsts.GetCpu().emissionFactor =
        //    Vector3(0.0f);
        //m_appBase->m_ground->m_materialConsts.GetCpu().metallicFactor = 0.f;
        //m_appBase->m_ground->m_materialConsts.GetCpu().roughnessFactor =  0.65f;

        //Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        //// m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
        ////                        Matrix::CreateTranslation(position));
        //m_appBase->m_ground->UpdateTranseform(info.scale, info.rotation,
        //                                      info.position);
        //// m_ground->useTessellation = true;

        // // m_appBase->m_mirrorPlane =
        // //   DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        //// m_mirror = m_ground; // 바닥에 거울처럼 반사 구현
        //  m_appBase->m_basicList.push_back(
        //      m_appBase->m_ground); // 거울은 리스트에 등록 X


          return make_shared<Model>();
}

shared_ptr< Model> JsonManager::CreateSphere(ObjectSaveInfo info) {
    auto meshes = GeometryGenerator::MakeSphere(1.f, 25, 25);
    shared_ptr<Model> tempModel = make_shared<Model>(
        m_appBase->m_device, m_appBase->m_context, vector{meshes});

    tempModel->UpdateTranseform(info.scale, info.rotation, info.position);
    tempModel->m_castShadow = true;

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


    return tempModel;
    //return make_shared<Model>();
}

shared_ptr<class Model> JsonManager::CreateTree(ObjectSaveInfo info) {
    string path = "../Assets/Foliage/Gledista_Triacanthos_FBX/";
    auto meshes = GeometryGenerator::ReadFromFile(
        path, "Gledista_Triacanthos_3.fbx", false);

    Vector3 center(0.0f, 0.0f, 2.0f);

    shared_ptr<Model> m_leaves =
        make_shared<Model>(m_appBase->m_device, m_appBase->m_context,
                                             vector{meshes[2], meshes[3]});
    m_leaves->m_meshConsts.GetCpu().windTrunk = 0.1f;
    m_leaves->m_meshConsts.GetCpu().windLeaves = 0.01f;
    m_leaves->m_materialConsts.GetCpu().albedoFactor = Vector3(0.3f);
    m_leaves->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
    m_leaves->m_materialConsts.GetCpu().metallicFactor = 0.2f;
     m_leaves->UpdateTranseform(info.scale, info.rotation, info.position      );

   m_appBase->m_basicList.push_back(m_leaves); // 리스트에 등록

    shared_ptr<Model>m_trunk = make_shared<Model>(m_appBase->m_device, m_appBase->m_context,
        vector{meshes[0], meshes[1],
               meshes[4]}); // Trunk and branches (4 is trunk)
    m_trunk->m_meshConsts.GetCpu().windTrunk = 0.1f;
    m_trunk->m_meshConsts.GetCpu().windLeaves = 0.0f;
    m_trunk->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
    m_trunk->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
    m_trunk->m_materialConsts.GetCpu().metallicFactor = 0.0f;
    m_trunk->UpdateTranseform(info.scale, info.rotation, info.position);
    m_trunk->SetChildModel(m_leaves);

    return m_trunk; 
}  
  
shared_ptr<class Model> JsonManager::CreateBillboadTree(ObjectSaveInfo info) {
    shared_ptr<BillboardModel> model = std::make_shared<BillboardModel>(m_appBase);

    // m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
    //                        1.0f, L"GameExplosionPS.hlsl");
    model->m_castShadow = false;
    model->Initialize(m_appBase->m_device, m_appBase->m_context, {{0.0f, 0.0f, 0.0f, 1.0f}}, 2.f, Graphics::billboardPS);
    model->UpdateTranseform(info.scale, info.rotation, info.position);

    
    return model;
}
} // namespace hlab