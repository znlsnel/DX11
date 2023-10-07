#pragma once
#include <string>
#include <sstream>
#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" 
#include "rapidjson/filereadstream.h"



namespace hlab {
using namespace std;

class JsonManager {
  public:
    JsonManager(){};
    JsonManager(class AppBase* appBase);
    ~JsonManager() {  };

    bool ParseJson(rapidjson::Document &doc, const std::string &jsonData);
    std::string JsonDocToString(rapidjson::Document &doc,
                                      bool isPretty = false);
    void TestJson_Parse();
    void TestJson_AddMember();

    class AppBase *m_appBase = nullptr;

    void LoadMesh();
   void SaveMesh();
    void CreateMesh(struct ObjectSaveInfo temp);

    shared_ptr<class Model> CreateCharacter(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateMountain(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateCylinder(struct ObjectSaveInfo info); 
    shared_ptr<class Model> CreateGround(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSphere(struct ObjectSaveInfo info);
    shared_ptr<class Model> CreateSquare(struct ObjectSaveInfo info);

   rapidjson::Document m_saveFile;
};
}
