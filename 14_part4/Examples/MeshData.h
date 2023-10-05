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
    float objectID = 0.0f;
    string meshName;

    Vector3 scale;
    Vector3 position;
    Vector3 rotation;
};

 enum meshID : int { 
	 character = 0,
	 floor = 1,
	 mountain = 2,
	 Square = 3,
	 box = 4,
	 Sphere = 5,
	 Cylinder = 6,
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