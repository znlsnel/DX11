#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace hlab {

// 일반적으로는 헤더파일에서 using namespace std를 사용하지 않습니다.
// 여기서는 강의 동영상에 녹화되는 코드 길이를 줄이기 위해서 사용하였습니다.
// 예: std::vector -> vector
using namespace glm;
using namespace std;

// 변환
struct Transformation {
    // 중요: 이것들을 하나로 합칠 순 없을까?
    vec3 scale = vec3(1.0f);
    vec3 translation = vec3(0.0f);
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
};

// 재질
struct Material {
    vec3 ambient = vec3(0.1f);
    vec3 diffuse = vec3(1.0f);
    vec3 specular = vec3(1.0f);
    float shininess;
};

// 조명
struct Light {
    vec3 strength = vec3(1.0f);
    vec3 direction = vec3(0.0f, -1.0f, 0.0f); // directional/spot light only

    // 아래 옵션들은 조명 강의에서 사용합니다.
    // vec3 position; // point/spot light only
    // float fallOffStart; // point/spot light only
    // float fallOffEnd;   // point/spot light only
    // float spotPower;    // spot light only
};

// 폴리곤 메쉬를 다루는 클래스
class Mesh {
  public:
    void InitBox();
    void CopyToBuffer();

  public:
    // CPU 메모리의 기하 정보
    vector<vec3> vertices;
    vector<size_t> indices;
    vector<vec3> normals;
    //vector<vec3> colors;// 색을 Shading으로 결정
    //vector<vec2> textureCoords; // Texture Coordinates

    // GPU 버퍼라고 생각합시다.
    vector<vec3> vertexBuffer;
    vector<vec3> normalBuffer;
    vector<size_t> indexBuffer;
    // vector<vec3> colorBuffer;
    // vector<vec2> uvBuffer;

    // 모든 버텍스에 공통으로 적용되는 변환(Transformations)
    Transformation transformation;

    // 재질(Material)
    Material material;
};
} // namespace hlab