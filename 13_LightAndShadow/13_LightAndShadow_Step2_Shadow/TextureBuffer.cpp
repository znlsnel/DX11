#include "TextureBuffer.h"

namespace hlab {

void TextureBuffer::Initialize(ComPtr<ID3D11Device> &device, const UINT width,
                               const UINT height, const DXGI_FORMAT format,
                               const UINT numQualityLevels,
                               const UINT sampleCount) {

    m_width = width;
    m_height = height;

    m_format = format;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Format = format; // DXGI_FORMAT_R16G16B16A16_FLOAT
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    desc.SampleDesc.Quality = numQualityLevels - 1;
    desc.SampleDesc.Count = sampleCount;

    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, m_buffer.GetAddressOf()));

    ThrowIfFailed(device->CreateShaderResourceView(m_buffer.Get(), NULL,
                                                   m_bufferSRV.GetAddressOf()));

    ThrowIfFailed(device->CreateRenderTargetView(m_buffer.Get(), NULL,
                                                 m_bufferRTV.GetAddressOf()));

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, m_resolvedBuffer.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(
        m_resolvedBuffer.Get(), NULL, m_resolvedSRV.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(m_resolvedBuffer.Get(), NULL,
                                                 m_resolvedRTV.GetAddressOf()));
}

void TextureBuffer::Resolve(ComPtr<ID3D11DeviceContext> &context) {
    context->ResolveSubresource(m_resolvedBuffer.Get(), 0, m_buffer.Get(), 0,
                                m_format);
}

} // namespace hlab