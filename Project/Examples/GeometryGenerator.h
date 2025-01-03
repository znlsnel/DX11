﻿#pragma once

#include <directxtk/SimpleMath.h>
#include <string>
#include <tuple>
#include <vector>

#include "AnimationClip.h"
#include "MeshData.h"
#include "Vertex.h"

namespace hlab {

using DirectX::SimpleMath::Vector2;
using std::string;
using std::tuple;

class GeometryGenerator {
  public:
    static vector<MeshData> ReadFromFile(string basePath, string filename,
                                         bool revertNormals = false, bool gltNormal = false);

    static auto ReadAnimationFromFile(string basePath, string filename,
                                      bool revertNormals = false)
        -> tuple<vector<MeshData>, AnimationData>;

    static void Normalize(const Vector3 center, const float longestLength,
                          vector<MeshData> &meshes, AnimationData &aniData);

    static MeshData MakeSquare(const float scale = 1.0f,
                               const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeSquareGrid(const int numSlices, const int numStacks,
                                   const float scale = 1.0f,
                                   const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeTestTessellation();
    static MeshData
    MakeTessellationPlane(const int numSlices,
                                          const int numStacks,
                                          const float scale,
                                          const Vector2 texScale,
                                        std::vector<uint8_t>& heightMap);
    static MeshData MakeGrass();
    static MeshData MakeBox(float scale = 1.0f);
    static MeshData MakeWireBox(const Vector3 center, const Vector3 extents);
    static MeshData MakeWireSphere(const Vector3 center, const float radius);
    static MeshData MakeCylinder(const float bottomRadius,
                                 const float topRadius, float height,
                                 int numSlices);
    static MeshData MakeSphere(float radius, const int numSlices,
                               const int numStacks,
                               const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeTetrahedron();
    static MeshData MakeIcosahedron();
    static MeshData SubdivideToSphere(const float radius, MeshData meshData);
    static MeshData MakeLine();

    static void SetVertexFromHeightMap(int mapWidth, int mapHeight, float scale, float dx, float dy, Vertex &v, std::vector<uint8_t>& heightMapImage);
};
} // namespace hlab
