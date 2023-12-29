#include "FoliageModel.h"

hlab::FoliageModel::FoliageModel(ComPtr<ID3D11Device> &device,
                                 ComPtr<ID3D11DeviceContext> &context,
                                 const string &basePath,
                                 const string &filename) {
         
}
 
hlab::FoliageModel::FoliageModel(ComPtr<ID3D11Device> &device,
                                 ComPtr<ID3D11DeviceContext> &context,
                                 const vector<MeshData> &meshes) {

}

/*
        Matrix tempRow = tempModel->m_worldRow;
        tempRow.Translation(Vector3(0.0f));
        Vector3 tempExtents = Vector3::Transform(
                tempModel->m_boundingBox.Extents, tempRow);

        tempModel->UpdatePosition(
                tempModel->GetPosition() +
                Vector3(0.0f, std::abs(tempExtents.y), 0.0f));
*/