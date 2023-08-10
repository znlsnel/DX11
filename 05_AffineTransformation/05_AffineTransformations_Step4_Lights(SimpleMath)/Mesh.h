#pragma once

#include <directxtk/SimpleMath.h>
#include <iostream>
#include <vector>

namespace hlab {

// 일반적으로는 헤더파일에서 using namespace std를 사용하지 않습니다.
// 여기서는 강의 동영상에 녹화되는 코드 길이를 줄이기 위해서 사용하였습니다.
// 예: std::vector -> vector
using namespace DirectX::SimpleMath;
using namespace std;

// 변환
struct Transformation {
    // 중요: 이것들을 하나로 합칠 순 없을까?
    Vector3 scale = Vector3(1.0f);
    Vector3 translation = Vector3(0.0f);
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
};

// 재질
struct Material {
    Vector3 ambient = Vector3(0.1f);
    Vector3 diffuse = Vector3(1.0f);
    Vector3 specular = Vector3(1.0f);
    float shininess;
};

// 조명
struct Light {
    Vector3 strength = Vector3(1.0f);
    Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
    Vector3 position = Vector3(0.0f, 1.0f, 0.5f);
    float fallOffStart = 0.0f;
    float fallOffEnd = 1.8f;
    float spotPower = 0.0f;
};

// 폴리곤 메쉬를 다루는 클래스
class Mesh {
  public:
    void InitBox();
    void CopyToBuffer();

  public:
    // CPU 메모리의 기하 정보
    vector<Vector3> vertices;
    vector<size_t> indices;
    vector<Vector3> normals;
    // vector<Vector3> colors;// 색을 Shading으로 결정
    // vector<Vector2> textureCoords; // Texture Coordinates

    // GPU 버퍼라고 생각합시다.
    vector<Vector3> vertexBuffer;
    vector<Vector3> normalBuffer;
    vector<size_t> indexBuffer;
    // vector<Vector3> colorBuffer;
    // vector<Vector2> uvBuffer;

    // 모든 버텍스에 공통으로 적용되는 변환(Transformations)
    Transformation transformation;

    // 재질(Material)
    Material material;
};
} // namespace hlab