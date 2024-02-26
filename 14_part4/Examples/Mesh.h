#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <iostream>
#include <vector>

#include <d3d11.h>
#include <vector>
#include <windows.h>
#include <wrl/client.h>

#include "Texture3D.h"

namespace hlab {
         
using Microsoft::WRL::ComPtr;

struct Mesh {

    public:
    Mesh(){};
    Mesh(Mesh *otherMesh) {  
		*this = *otherMesh; 
      }
    void SetLod(int level) {
	    level = std::clamp(level, 0, int(vertexBuffers.size()) - 1);
        vertexBuffer = vertexBuffers[level];
        vertexCount = vertexCounts[level];
        indexBuffer = indexBuffers[level];
        indexCount = indexCounts[level];
    };
       
    vector<ComPtr<ID3D11Buffer>> vertexBuffers;
    ComPtr<ID3D11Buffer> vertexBuffer;
    vector<ComPtr<ID3D11Buffer>> indexBuffers;
    ComPtr<ID3D11Buffer> indexBuffer;
     
    ComPtr<ID3D11Buffer> mergeVertexBuffer;
    ComPtr<ID3D11Buffer> mergeIndexBuffer;

    ComPtr<ID3D11Buffer> meshConstsGPU;
    ComPtr<ID3D11Buffer> materialConstsGPU;

    ComPtr<ID3D11Texture2D> albedoTexture;
    ComPtr<ID3D11Texture2D> emissiveTexture;
    ComPtr<ID3D11Texture2D> normalTexture;
    ComPtr<ID3D11Texture2D> heightTexture;
    ComPtr<ID3D11Texture2D> aoTexture;
    ComPtr<ID3D11Texture2D> metallicRoughnessTexture;
    ComPtr<ID3D11Texture2D> artTexture;
    ComPtr<ID3D11Texture2D> billboardDiffuseTexture;
    ComPtr<ID3D11Texture2D> billboardNormalTexture;
    ComPtr<ID3D11Texture2D> billboardArtTexture;

    ComPtr<ID3D11ShaderResourceView> albedoSRV;
    ComPtr<ID3D11ShaderResourceView> emissiveSRV;
    ComPtr<ID3D11ShaderResourceView> normalSRV;
    ComPtr<ID3D11ShaderResourceView> heightSRV;
    ComPtr<ID3D11ShaderResourceView> aoSRV;
    ComPtr<ID3D11ShaderResourceView> metallicRoughnessSRV;
    ComPtr<ID3D11ShaderResourceView> artSRV;
    ComPtr<ID3D11ShaderResourceView> billboardDiffuseSRV;
    ComPtr<ID3D11ShaderResourceView> billboardNormalSRV;
    ComPtr<ID3D11ShaderResourceView> billboardArtSRV;
    // 3D Textures 
    Texture3D densityTex;
    Texture3D lightingTex;

    vector<UINT> indexCounts = {};
    UINT indexCount = 0; 
    vector<UINT> vertexCounts = {};
    UINT vertexCount = 0;
    UINT mergeVertexCount = 0;
    UINT mergeIndexCount = 0; 
    UINT stride = 0;
    UINT offset = 0;

    
}; 

} // namespace hlab
   