#include "TessellatedQuad.h"

namespace hlab {

void TessellatedQuad::Initialize(ComPtr<ID3D11Device> &device) {

    // Sampler �����
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

    // Vetex Buffer �����
    std::vector<Vector4> controlPoints = {{-1.0f, 1.0f, 0.0, 1.0f},
                                          {1.0f, 1.0f, 0.0, 1.0f},
                                          {-1.0f, -1.0f, 0.0, 1.0f},
                                          {1.0f, -1.0f, 0.0, 1.0f}};
    for (auto &cp : controlPoints) {
        cp.x *= 0.5f;
        cp.y = cp.y * 0.5f + 0.5f;
        cp.z = 1.5f; // �ʱ� ��ġ ����
    }
    D3D11Utils::CreateVertexBuffer(device, controlPoints, m_vertexBuffer);

    m_indexCount = uint32_t(controlPoints.size());

    D3D11Utils::CreateConstBuffer(device, m_constantData, m_constantBuffer);

    // ���̴� �ʱ�ȭ
    vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, // Vector4
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    D3D11Utils::CreateVertexShaderAndInputLayout(
        device, L"TessellatedQuadVS.hlsl", inputElements, m_vertexShader,
        m_inputLayout);

    D3D11Utils::CreateHullShader(device, L"TessellatedQuadHS.hlsl",
                                 m_hullShader);

    D3D11Utils::CreateDomainShader(device, L"TessellatedQuadDS.hlsl",
                                   m_domainShader);

    // ���̵��� �ϱ� ���ؼ� BasicPixelShader ���
    D3D11Utils::CreatePixelShader(device, L"TessellatedQuadPS.hlsl",
                                  m_pixelShader);

    // �ؽ��� �ʱ�ȭ
    // D3D11Utils::CreateTextureArray(device, filenames, m_texArray,
    //                               m_texArraySRV);
}

void TessellatedQuad::Render(ComPtr<ID3D11DeviceContext> &context) {

    context->IASetInputLayout(m_inputLayout.Get());
    UINT stride = sizeof(Vector4); // sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride,
                                &offset);
    context->VSSetShader(m_vertexShader.Get(), 0, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShader(m_pixelShader.Get(), 0, 0);
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    ID3D11ShaderResourceView *resViews[3] = {m_texArraySRV.Get(),
                                             m_diffuseResView.Get(),
                                             m_specularResView.Get()};
    context->PSSetShaderResources(0, 3, resViews);

    // Hull shader
    context->HSSetShader(m_hullShader.Get(), 0, 0);
    context->HSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // Domain shader
    context->DSSetShader(m_domainShader.Get(), 0, 0);
    context->DSSetSamplers(0, 1, m_samplerState.GetAddressOf());
    context->DSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // ���������� 4���� Control Point�� ����
    context->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    context->Draw(m_indexCount, 0);

    // HS, DS�� ������� �ʴ� �ٸ� ��ü���� ���� nullptr�� ����
    context->HSSetShader(NULL, 0, 0);
    context->DSSetShader(NULL, 0, 0);
}

} // namespace hlab