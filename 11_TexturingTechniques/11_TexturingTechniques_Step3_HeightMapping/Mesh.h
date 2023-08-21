#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <iostream>
#include <vector>

#include <d3d11.h>
#include <vector>
#include <windows.h>
#include <wrl/client.h>

namespace hlab {

using Microsoft::WRL::ComPtr;

struct Mesh {
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    ComPtr<ID3D11Buffer> vertexConstantBuffer;
    ComPtr<ID3D11Buffer> pixelConstantBuffer;

    ComPtr<ID3D11Texture2D> albedoTexture;
    ComPtr<ID3D11Texture2D> normalTexture;
    ComPtr<ID3D11Texture2D> heightTexture;
    ComPtr<ID3D11Texture2D> aoTexture; // Ambient Occlusion
    ComPtr<ID3D11ShaderResourceView> albedoTextureResourceView;
    ComPtr<ID3D11ShaderResourceView> normalTextureResourceView;
    ComPtr<ID3D11ShaderResourceView> heightTextureResourceView;
    ComPtr<ID3D11ShaderResourceView> aoTextureResourceView;

    UINT m_indexCount = 0;
    UINT m_vertexCount = 0;
};
} // namespace hlab
