#include <d3d11.h>
#include <iostream>
#include <memory>
#include <windows.h>
#include <wrl.h>

using namespace std;

int main() {

    // Component Object Model (COM)

    // Microsoft::WRL::ComPtr is a C++ template smart-pointer for COM objects
    // that is used extensively in Windows Runtime (WinRT) C++ programming
    // https://github.com/Microsoft/DirectXTK/wiki/ComPtr
    Microsoft::WRL::ComPtr<ID3D11Device> device; // COM interface
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

    // 비교: std::shared_ptr<ID3D11Device> device = make_shared<ID3D11Device>(...);
    // 비교: ID3D11Device *device = nullptr;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,  D3D_FEATURE_LEVEL_9_1};

    D3D_FEATURE_LEVEL m_d3dFeatureLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                  // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics
                                  // driver.
        0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags, // Set debug and Direct2D compatibility flags.
        featureLevels, // List of feature levels this app can support.
        ARRAYSIZE(featureLevels), // Size of the list above.
        D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Microsoft
                           // Store apps.
        &device,           // Returns the Direct3D device created.
        &m_d3dFeatureLevel, // Returns feature level of device created.
        &context            // Returns the device immediate context.
    );

    if (FAILED(hr)) {
        cout << "Failed." << endl;
        return -1;
    }

    using Microsoft::WRL::ComPtr;

    // 인터페이스에 대한 포인터를 가지고 있을 다른 ComPtr 변수
    ComPtr<ID3D11Device> m_d3dDevice;

    hr = device.As(&m_d3dDevice);

    if (FAILED(hr)) {
        cout << "Failed." << endl;
        return -1;
    }

    auto temp = m_d3dDevice.Get();

    // 수동으로 Release
    m_d3dDevice.Reset();
    // m_d3dDevice = nullptr;

    if (!m_d3dDevice) {
        cout << "m_d3dDevice Released" << endl;
    }

    /*
    ID3D11Buffer *pptr[1] = {
        m_constantBuffer.Get(),
    };
    m_context->VSSetConstantBuffers(0, 1, pptr);
    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    */

    return 0;
}
