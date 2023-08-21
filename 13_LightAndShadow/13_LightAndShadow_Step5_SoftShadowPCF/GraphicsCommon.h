#pragma once

#include "D3D11Utils.h"
#include "GraphicsPSO.h"

namespace hlab {

// ����: DirectX_Graphic-Samples �̴Ͽ���
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCommon.h

namespace Graphics {

// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> linearClampSS;
extern ComPtr<ID3D11SamplerState> shadowPointSS;
extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
extern ComPtr<ID3D11RasterizerState> solidRS;
extern ComPtr<ID3D11RasterizerState> solidCCWRS; // Counter-ClockWise
extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCCWRS;
extern ComPtr<ID3D11RasterizerState> postProcessingRS;

// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS; // �Ϲ������� �׸���
extern ComPtr<ID3D11DepthStencilState> maskDSS; // ���ٽǹ��ۿ� ǥ��
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // ���ٽ� ǥ�õ� ����

// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11VertexShader> samplingVS;
extern ComPtr<ID3D11VertexShader> normalVS;
extern ComPtr<ID3D11VertexShader> depthOnlyVS;
extern ComPtr<ID3D11PixelShader> basicPS;
extern ComPtr<ID3D11PixelShader> skyboxPS;
extern ComPtr<ID3D11PixelShader> combinePS;
extern ComPtr<ID3D11PixelShader> bloomDownPS;
extern ComPtr<ID3D11PixelShader> bloomUpPS;
extern ComPtr<ID3D11PixelShader> normalPS;
extern ComPtr<ID3D11PixelShader> depthOnlyPS;
extern ComPtr<ID3D11PixelShader> postEffectsPS;
extern ComPtr<ID3D11GeometryShader> normalGS;

// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> samplingIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;
extern ComPtr<ID3D11InputLayout> postProcessingIL;

// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;

// Graphics Pipeline States
extern GraphicsPSO defaultSolidPSO;
extern GraphicsPSO defaultWirePSO;
extern GraphicsPSO stencilMaskPSO;
extern GraphicsPSO reflectSolidPSO;
extern GraphicsPSO reflectWirePSO;
extern GraphicsPSO mirrorBlendSolidPSO;
extern GraphicsPSO mirrorBlendWirePSO;
extern GraphicsPSO skyboxSolidPSO;
extern GraphicsPSO skyboxWirePSO;
extern GraphicsPSO reflectSkyboxSolidPSO;
extern GraphicsPSO reflectSkyboxWirePSO;
extern GraphicsPSO normalsPSO;
extern GraphicsPSO depthOnlyPSO;
extern GraphicsPSO postEffectsPSO;
extern GraphicsPSO postProcessingPSO;

void InitCommonStates(ComPtr<ID3D11Device> &device);

// ���������� InitCommonStates()���� ���
void InitSamplers(ComPtr<ID3D11Device> &device);
void InitRasterizerStates(ComPtr<ID3D11Device> &device);
void InitBlendStates(ComPtr<ID3D11Device> &device);
void InitDepthStencilStates(ComPtr<ID3D11Device> &device);
void InitPipelineStates(ComPtr<ID3D11Device> &device);
void InitShaders(ComPtr<ID3D11Device> &device);

} // namespace Graphics

} // namespace hlab
