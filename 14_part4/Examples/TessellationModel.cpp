#include "TessellationModel.h"
#include "AppBase.h" 
#include "GraphicsPSO.h"
#include <filesystem>

namespace hlab { 

hlab::TessellationModel::TessellationModel(ComPtr<ID3D11Device> &device,
                                           ComPtr<ID3D11DeviceContext> &context,
    const vector<MeshData> &meshes, AppBase* appBase, bool Plane) {
    m_BVHMaxLevel = 20;
    m_appBase = appBase;
    Model::Initialize(device, context, meshes);

    bool hasTextureMap = false; 
    auto filePath = std::filesystem::current_path();
    string heightMapPath;
    for (const auto file : std::filesystem::directory_iterator(filePath)) {
        if (file.path().stem() == "textureMap") {
            hasTextureMap = true;
            heightMapPath = file.path().string();
            break;
        }
    }
    if (hasTextureMap) {
        D3D11Utils::CreateTexture(device, context, heightMapPath, false,
                                  m_textureMapBuffer, m_textureMapSRV);
    } else 
        D3D11Utils::CreateTexture(device, context, m_textureMapBuffer,
                                  m_textureMapSRV, false, 1024, 1024, 3);

        D3D11_TEXTURE2D_DESC desc;
    m_textureMapBuffer->GetDesc(&desc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    uavDesc.Format = desc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    ThrowIfFailed(device->CreateUnorderedAccessView(
        m_textureMapBuffer.Get(), &uavDesc, m_textureMapUAV.GetAddressOf()));


    //ThrowIfFailed(device->CreateUnorderedAccessView(
    //    m_textureMapBuffer.Get(), NULL,
    //        m_textureMapUAV.GetAddressOf())); 
    // 
    //string BasePath = "../Assets/Textures/PBR/TerrainTextures/";
    //string Ground37 =  "Ground037_2K-PNG";
    //string Ground64 =   "Ground063_2K-PNG";
    //string PavingStones070 =   "PavingStones070_2K-PNG";
    //string Rock030 =   "Rock030_2K-PNG";
    string BasePath = "../../Assets/Surfaces/"; 
    string Ground37 = "s1/T_Forest_Ground_Dried_Leaves_xeukeip_2K";
    string Ground64 = "s2/T_Fine_Asphalt_vlzobiady_2K";
    string PavingStones070 = "s5/T_Rocky_Ground_vl0fdfho_2K";
    string Rock030 = "s6/T_Thai_Rippled_Sand_td1hcimn_2K";
       
    vector<string> albedoTextureFilenames;
    vector<string> aoTextureFilename;
    vector<string> normalTextureFilename;
    vector<string> heightTextureFilename;
    vector<string> ORDpTextureFilename;
     
     
    auto CraeteTextureArray = [&](string &name) {
        string path = BasePath + name + "/";
        albedoTextureFilenames.push_back(path + name + "_Color.png");
        aoTextureFilename.push_back(path + name + "_AmbientOcclusion.png");
        normalTextureFilename.push_back(path + name + "_NormalDX.png");
        heightTextureFilename.push_back(path + name + "_Displacement.png");
    };
    //  

   //string surfaceBasePath = "C:\\DEVELOPMENT\\GIT\\DX11_HongLab\\Assets\\Surfaces\\";
    auto CraeteSurfaceTextureArray = [&](string &name) {
       albedoTextureFilenames.push_back(BasePath + name + "_D.HDR");
       normalTextureFilename.push_back(BasePath + name + "_N.HDR");
       ORDpTextureFilename.push_back(BasePath + name + "_ORDp.HDR");
    };     
      
    CraeteSurfaceTextureArray(Ground37);
    CraeteSurfaceTextureArray(Ground64);
    CraeteSurfaceTextureArray(PavingStones070);
    CraeteSurfaceTextureArray(Rock030);
     
    D3D11Utils::CreateTextureArray(device, context, albedoTextureFilenames,
                                   m_albedoTexturesBuffer, m_albedoTexturesSRV);

    D3D11Utils::CreateTextureArray(device, context, normalTextureFilename,
                                   m_normalTexturesBuffer, m_normalTexturesSRV);
    //D3D11Utils::CreateTextureArray(device, context, aoTextureFilename,
    //                               m_aoTexturesBuffer, m_aoTexturesSRV);
    //D3D11Utils::CreateTextureArray(device, context, heightTextureFilename,
    //                               m_heightTexturesBuffer, m_heightTexturesSRV);
    D3D11Utils::CreateTextureArray(device, context, heightTextureFilename,
                                   m_ORDpTexturesBuffer, m_ORDpTexturesSRV); 


    m_csConsts.GetCpu().pos = {0.0f, 0.0f}; 
    m_csConsts.GetCpu().radius = 100.0f;
    m_csConsts.GetCpu().type = 3.0f / 255.f; 
    m_csConsts.Initialize(device);
}    

TessellationModel::~TessellationModel() {
         
        if (false) 
                return;

    D3D11_TEXTURE2D_DESC desc;

    m_textureMapBuffer->GetDesc(&desc);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = 0;

    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ComPtr<ID3D11Texture2D> tempTexture;
    ThrowIfFailed(m_appBase->m_device->CreateTexture2D(
        &desc, NULL, tempTexture.GetAddressOf()));
     
    ID3D11Resource *uavTexture = nullptr;
    m_textureMapUAV->GetResource(&uavTexture); 
    m_appBase->m_context->ResolveSubresource(tempTexture.Get(), 0, uavTexture,  0, DXGI_FORMAT_R8G8B8A8_UNORM);
      

     D3D11Utils::WriteToPngFile(m_appBase->m_device, m_appBase->m_context, tempTexture,
                              "textureMap.png");
} 

void hlab::TessellationModel::RenderTessellation(
    ComPtr<ID3D11DeviceContext> &context, bool tessellation) {
    if (tessellation)
        TessellationModel::Render(context);
    else
        Model::Render(context);
}

void hlab::TessellationModel::Render(ComPtr<ID3D11DeviceContext> &context) {
         
        if (renderState == ERenderState::reflect) {
    
                Model::Render(context);
                return;
        } 
    if (m_isVisible) {
        for (const auto &mesh : m_meshes) {
            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            vector<ID3D11ShaderResourceView *> resViews = {
                m_albedoTexturesSRV.Get(), m_normalTexturesSRV.Get(),
                m_ORDpTexturesSRV.Get(), m_textureMapSRV.Get()};
                
            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);
            context->VSSetConstantBuffers(1, 2, constBuffers);

            context->HSSetConstantBuffers(1, 2, constBuffers);

                   
            ID3D11ShaderResourceView *temp[2] = { 
                    m_textureMapSRV.Get(),
                m_ORDpTexturesSRV.Get(),
            };
            
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
    renderState = ERenderState::basic;

    return wired ? Graphics::terrainWirePSO : Graphics::terrainSolidPSO;
    // TODO: 여기에 return 문을 삽입합니다.   
}   
   
GraphicsPSO &TessellationModel::GetDepthOnlyPSO() { 
    renderState = ERenderState::depth;   
    return Graphics::terrainDepthPSO;
      
} 
    
void TessellationModel::UpdateTextureMap(
    ComPtr<ID3D11DeviceContext> &context, Vector3 pos, int type) { 
                if (type == -1)     
                return;         
        pos -= GetPosition();      
       // 0 ~ 60 -> 0 ~ 1024     
        // 0 1       
        // 2 3   
        float x = std::clamp((pos.x + 30.f) * (1024 / 60.f), 0.0f, 1024.0f);
        float y = std::clamp((pos.z + 30.f) * (1024 / 60.f), 0.0f, 1024.0f); 
            
        m_csConsts.GetCpu().radius = editRadius; 
        m_csConsts.GetCpu().type = (float)type / 255.0f;
        m_csConsts.GetCpu().pos = Vector2(x, y); 
                 
        m_csConsts.Upload(context);      
                  
        m_appBase->SetPipelineState(Graphics::editTexturePSO);
     
        int width = 1024;    
        int height = 1024;     
        UINT X = UINT(ceil(width / 32));  
        UINT Y = UINT(ceil(height / 32));   
        context->CSSetConstantBuffers(0, 1, m_csConsts.m_gpu.GetAddressOf());
        context->CSSetUnorderedAccessViews(
            0, 1, m_textureMapUAV.GetAddressOf(), NULL);
        context->Dispatch(X, Y, 1);  
       m_appBase->ComputeShaderBarrier();  
               
       ID3D11Resource *srvTexture = nullptr;
       ID3D11Resource *uavTexture = nullptr;

       m_textureMapSRV->GetResource(&srvTexture);
       m_textureMapUAV->GetResource(&uavTexture);
       context->CopyResource(srvTexture, uavTexture);
       context->CopyResource(m_textureMapBuffer.Get(), uavTexture);
}    
    
} // namespace hlab    