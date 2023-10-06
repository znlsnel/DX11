#pragma once


#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "Vertex.h"


namespace hlab {

using std::string;
using std::vector;


 struct ObjectSaveInfo {

    int meshID = 0;

    Vector3 scale{1.f};
    Vector3 position{0.f};
    Vector3 rotation{0.f};

    string meshName;
    float objectID;
 };

 enum meshID : int { 
	 ECharacter = 0,
	 EGround = 1,
	 EMountain = 2,
	 ESquare = 3,
	 EBox = 4,
	 ESphere = 5,
	 ECylinder = 6,
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

};

} // namespace hlab