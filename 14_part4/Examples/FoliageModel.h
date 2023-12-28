#pragma once

#include "Model.h"

namespace hlab {
class FoliageModel : public Model{
  public:
    FoliageModel(){};
    FoliageModel(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context, const string &basePath,
                 const string &filename);
    FoliageModel(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context,
                 const vector<MeshData> &meshes);

     
};
}
