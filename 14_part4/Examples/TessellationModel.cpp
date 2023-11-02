#include "TessellationModel.h"
#include "AppBase.h"
#include "GraphicsPSO.h"
#include <filesystem>

namespace hlab { 

hlab::TessellationModel::TessellationModel(ComPtr<ID3D11Device> &device,
                                           ComPtr<ID3D11DeviceContext> &context,
                                           const vector<MeshData> &meshes) {
    Model::Initialize(device, context, meshes);

    bool hasHeightMap = false;
    auto filePath = std::filesystem::current_path();
    string heightMapPath;
    for (const auto file : std::filesystem::directory_iterator(filePath)) {
        if (file.path().stem() == "heightMap") {
            hasHeightMap = true;
            heightMapPath = file.path().string();
            break;
        }
    }
    if (hasHeightMap) {
        D3D11Utils::CreateTexture(device, context, heightMapPath, true,
                                  m_heightMapTexture, m_heightMapSRV);
    } else
        D3D11Utils::CreateTexture(device, context, m_heightMapTexture,
                                  m_heightMapSRV, true, 1000, 1000);
}

void hlab::TessellationModel::RenderTessellation(
    ComPtr<ID3D11DeviceContext> &context, bool tessellation) {
    if (tessellation)
        TessellationModel::Render(context);
    else
        Model::Render(context);
}

void hlab::TessellationModel::Render(ComPtr<ID3D11DeviceContext> &context) {
         
    if (m_isVisible) {
        for (const auto &mesh : m_meshes) {

            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);
            context->VSSetConstantBuffers(1, 2, constBuffers);

            context->HSSetConstantBuffers(1, 2, constBuffers);


            ID3D11ShaderResourceView *temp[2] = {mesh->heightSRV.Get(),
                                                 m_heightMapSRV.Get()};
            context->DSSetShaderResources(0, 2, temp);
            context->DSSetConstantBuffers(1, 2, constBuffers);

            // Volume Rendering
            context->PSSetConstantBuffers(1, 2, constBuffers);
            context->PSSetShaderResources(0, // register(t0)
                                          UINT(resViews.size()),
                                          resViews.data());
            if (mesh->densityTex.GetSRV())
                context->PSSetShaderResources(
                    5, 1, mesh->densityTex.GetAddressOfSRV());
            if (mesh->lightingTex.GetSRV())
                context->PSSetShaderResources(
                    6, 1, mesh->lightingTex.GetAddressOfSRV());

            

            context->Draw(mesh->vertexCount, 0);

            // Release resources
            ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
            context->PSSetShaderResources(5, 3, nulls);
        }
    }
}

GraphicsPSO &hlab::TessellationModel::GetPSO(const bool wired) {

        currPSO =  wired ? Graphics::terrainWirePSO : Graphics::terrainSolidPSO;
    return currPSO;
    // TODO: 여기에 return 문을 삽입합니다.
}

} // namespace hlab