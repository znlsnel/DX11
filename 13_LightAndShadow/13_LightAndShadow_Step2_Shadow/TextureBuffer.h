#pragma once

#include "D3D11Utils.h"

namespace hlab {

class TextureBuffer {
  public:
    void Initialize(ComPtr<ID3D11Device> &device, const UINT width,
                    const UINT height, const DXGI_FORMAT format,
                    const UINT numQualityLevels, const UINT sampleCount);

    void Resolve(ComPtr<ID3D11DeviceContext> &context);

    ComPtr<ID3D11Texture2D> m_buffer;
    ComPtr<ID3D11Texture2D> m_resolvedBuffer;
    ComPtr<ID3D11RenderTargetView> m_bufferRTV;
    ComPtr<ID3D11RenderTargetView> m_resolvedRTV;
    ComPtr<ID3D11ShaderResourceView> m_bufferSRV;
    ComPtr<ID3D11ShaderResourceView> m_resolvedSRV;

    UINT m_width = 0;
    UINT m_height = 0;

  private:
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
};

} // namespace hlab