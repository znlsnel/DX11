#pragma once
#include <sstream>
#include <iostream>
#include <map>
#include <filesystem>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" 
#include "rapidjson/filereadstream.h"

#include "MeshData.h"

namespace hlab {
using namespace std;

struct QuicellMeshPathInfo {
    string mesh;
    string Diffuse; // D
    string Normal; // N 
    string Occlusion; // O
    string Roughness; // R
    string Displacement; // dp
    string metallic; // F
        };

class JsonManager {
  public:
    JsonManager(){};
    JsonManager(class AppBase* appBase);
    ~JsonManager()  {};

    bool ParseJson(rapidjson::Document &doc, const std::string &jsonData);
    std::string JsonDocToString(rapidjson::Document &doc,
                                      bool isPretty = false);

    class AppBase *m_appBase = nullptr;

    void LoadObjectPathInFolder();
    void SearchQuicellModels(const filesystem::path &directory, int count = 0);
    void SearchModelFiles(const filesystem::path& directory);

    void LoadMesh();
   void SaveMesh();
    void CreateMesh(struct ObjectSaveInfo temp);

    int objectID = 1;

    shared_ptr<class Model> CreateModel(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateQuicellModel(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateCharacter(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateMountain(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateCylinder(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreatePlane(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSphere(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSquare(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateBox(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateTree(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateBillboadTree(struct ObjectSaveInfo info);

    map<meshID, std::string> objectInfo = {
        {meshID::ESphere, "Sphere"},
        {meshID::EBox, "Box"},
        {meshID::ETree, "Tree"},
        {meshID::EBillboardTree, "BillboardTree"},
    };

    map < string, pair<string, string>> meshPaths;
    map<string, QuicellMeshPathInfo> quicellPaths;
    

    string modelsPath;
    string quicellPath;

   rapidjson::Document m_saveFile;
};
}
