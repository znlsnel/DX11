#pragma once
#include "Model.h"
#include "GraphicsPSO.h"

namespace hlab {
class TessellationModel : public Model{

  public:
        TessellationModel(){};
        TessellationModel(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                          const vector<MeshData> &meshes, class AppBase* appBase,  bool isPlane = false);
        ~TessellationModel();
        void RenderTessellation(ComPtr<ID3D11DeviceContext> &context,
                    bool tessellation = false);
        virtual void Render(ComPtr<ID3D11DeviceContext> &context) override;
        virtual GraphicsPSO &GetPSO(const bool wired)override;
        virtual GraphicsPSO &GetDepthOnlyPSO() override;
        //virtual GraphicsPSO &GetReflectPSO(const bool wired) override;
        float editRadius = 100.0f;
        void UpdateTextureMap(ComPtr<ID3D11DeviceContext>& context, Vector3 pos, int type);
         
private:
        class AppBase *m_appBase = nullptr;

        ComPtr<ID3D11UnorderedAccessView> m_textureMapUAV;
        ComPtr<ID3D11ShaderResourceView> m_textureMapSRV;
        ComPtr<ID3D11ShaderResourceView> m_albedoTexturesSRV;
        ComPtr<ID3D11ShaderResourceView> m_aoTexturesSRV;
        ComPtr<ID3D11ShaderResourceView> m_normalTexturesSRV;
        ComPtr<ID3D11ShaderResourceView> m_heightTexturesSRV;

        ComPtr<ID3D11Texture2D> m_textureMapBuffer;
        ComPtr<ID3D11Texture2D> m_albedoTexturesBuffer;
        ComPtr<ID3D11Texture2D> m_aoTexturesBuffer;
        ComPtr<ID3D11Texture2D> m_normalTexturesBuffer;
        ComPtr<ID3D11Texture2D> m_heightTexturesBuffer;

       ConstantBuffer<EditTextureConsts> m_csConsts;
};
}
