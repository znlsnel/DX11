#pragma once
#include "Model.h"
#include "GraphicsPSO.h"

namespace hlab {
class TessellationModel : public Model{
  public:
        TessellationModel(){};
        TessellationModel(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const vector<MeshData> &meshes);

        void RenderTessellation(ComPtr<ID3D11DeviceContext> &context,
                    bool tessellation = false);
        virtual void Render(ComPtr<ID3D11DeviceContext> &context) override;
        virtual GraphicsPSO &GetPSO(const bool wired)override;
        //virtual GraphicsPSO &GetDepthOnlyPSO() override;
        //virtual GraphicsPSO &GetReflectPSO(const bool wired) override;

private:
        ComPtr<ID3D11ShaderResourceView> m_heightMapSRV;
        ComPtr<ID3D11Texture2D> m_heightMapTexture;
};
}
