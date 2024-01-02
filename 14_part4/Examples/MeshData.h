#pragma once
#include <string>
#include "Vertex.h"


namespace hlab {

using std::string;
using std::vector;


 struct ObjectSaveInfo {

    int meshID = 0;

    Vector3 scale{1.f};
    Vector3 position{0.f};
    Vector3 rotation{0.f};

    string meshName = "";
    string meshPath = "";
    string previewPath = "";
    string quicellPath = "";
    int objectID;
    int quixelID = 0;
     
    float metallic = 1.0f; 
    float roughness = 1.0f;
    float minMetallic = 0.0f;
    float minRoughness = 0.0f;

    bool isFolige = false;
    float foliageRange = 0.0f;
    float foliageDensity = 0.0f;
 }; 
  
 enum meshID : int {
	 EQuicellFoliage = -3,
	 EQuicellPath = -2,
	ELoadToPath = -1,
	 ECharacter = 0,
	 EPlane = 1,
	 EMountain = 2,
	 ESquare = 3,
	 EBox = 4,
	 ESphere = 5,
	 ECylinder = 6,
	 ETree = 7,
	 EBillboardTree = 8,
 };
  
struct MeshData {
    vector<Vertex> vertices;
    vector<SkinnedVertex> skinnedVertices;
    vector<uint32_t> indices;
    string albedoTextureFilename;
    string emissiveTextureFilename;
    string normalTextureFilename;
    string heightTextureFilename; 
    string aoTextureFilename; // Ambient Occlusion
    string metallicTextureFilename;
    string roughnessTextureFilename;
    string opacityTextureFilename;
    string artTextureFilename;
    string billboardDiffuseTextureFilename;
    string billboardNormalTextureFilename;
    string billboardARTTextureFilename;
     
};

} // namespace hlab