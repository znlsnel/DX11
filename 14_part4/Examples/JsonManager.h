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
    void SearchModelFiles(const filesystem::path& directory);

    void LoadMesh();
   void SaveMesh();
    void CreateMesh(struct ObjectSaveInfo temp);

    shared_ptr<class Model> CreateModel(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateCharacter(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateMountain(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateCylinder(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreatePlane(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSphere(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSquare(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateBox(struct ObjectSaveInfo info);

    map<meshID, std::string> objectInfo = {
            {meshID::ESphere, "Sphere"},
                {meshID::EBox, "Box"}
    };

    map < string, pair<string, string>> meshPaths;
    string modelsPath;

   rapidjson::Document m_saveFile;
};
}
