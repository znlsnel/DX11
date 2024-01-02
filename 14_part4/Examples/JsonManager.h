#pragma once
#include <sstream>
#include <iostream>
#include <map>
#include <filesystem>
#include "D3D11Utils.h"


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" 
#include "rapidjson/filereadstream.h"

#include "MeshData.h"

namespace hlab {
using namespace std;

struct QuicellMeshPathInfo {
    vector<string> mesh;
    string Diffuse; // D
    string Normal; // N 
    string Occlusion; // O
    string Roughness; // R
    string Displacement; // dp
    string metallic; // F
    string art;
    string billboardDiffuse;
    string billboardNormal; 
    string billboardArt;

    vector<vector<MeshData>> meshs;
    vector<Vector3> yOffset;
    vector<bool> hasMeshs;

    ComPtr<ID3D11Texture2D> objectImage;
    ComPtr<ID3D11ShaderResourceView> objectImageSRV;
     
    bool isFolige = false; 
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
    shared_ptr<class Model> CreateMesh(struct ObjectSaveInfo temp);

    shared_ptr<class Model> CreateModel(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateQuicellModel(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateQuicellFoliageModel(struct ObjectSaveInfo info);
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



/*

[
  {
    "meshID": 0,
    "filePath": {
      "object": "Character",
      "objectPath": "",
      "screenshotPath": "",
      "quicellPath": ""
    },
    "scale": {
      "x": 0.20999999344348907,
      "y": 0.20000000298023224,
      "z": 0.20000000298023224
    },
    "position": {
      "x": -12.922481536865234,
      "y": 0.8529412150382996,
      "z": 4.398019790649414
    },
    "rotation": {
      "x": 0.0,
      "y": 47.59541320800781,
      "z": 0.0
    },
    "material": {
      "metallic": 1.0,
      "roughness": 1.0,
      "minMetallic": 0.0,
      "minRoughness": 0.0
    }
  },
  {
    "meshID": 7,
    "filePath": {
      "object": "Tree",
      "objectPath": "",
      "screenshotPath": "",
      "quicellPath": ""
    },
    "scale": {
      "x": 1.0,
      "y": 1.0,
      "z": 1.0
    },
    "position": {
      "x": -1.4351142644882202,
      "y": 1.6596574783325195,
      "z": -13.894312858581543
    },
    "rotation": {
      "x": 0.0,
      "y": 0.0,
      "z": 0.0
    },
    "material": {
      "metallic": 1.0,
      "roughness": 1.0,
      "minMetallic": 0.0,
      "minRoughness": 0.0
    }
  }
]
*/