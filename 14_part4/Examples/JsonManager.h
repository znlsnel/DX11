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

class JsonManager {

  public:
    JsonManager(){};
    JsonManager(class AppBase* appBase);

    bool ParseJson(rapidjson::Document &doc, const std::string &jsonData);
    std::string JsonDocToString(rapidjson::Document &doc,
                                      bool isPretty = false);
    void TestJson_Parse();
    void TestJson_AddMember();

    class AppBase *m_appBase = nullptr;

    void LoadMesh();
   void SaveMesh();

   rapidjson::Document m_saveFile;
};
}
