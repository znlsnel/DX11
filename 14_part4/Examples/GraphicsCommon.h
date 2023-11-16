#pragma once

#include "ComputePSO.h"
#include "D3D11Utils.h"
#include "GraphicsPSO.h"

namespace hlab {

// ����: DirectX_Graphic-Samples �̴Ͽ���
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCommon.h

namespace Graphics {

// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> tessellationSS;
extern ComPtr<ID3D11SamplerState> linearClampSS;
extern ComPtr<ID3D11SamplerState> pointClampSS;
extern ComPtr<ID3D11SamplerState> shadowPointSS;
extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern ComPtr<ID3D11SamplerState> pointWrapSS;
extern ComPtr<ID3D11SamplerState> linearMirrorSS;
extern vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
// CCW: Counter-Clockwise (�ݽð� ������ �ǹ��մϴ�.)
extern ComPtr<ID3D11RasterizerState> solidRS; // front only
extern ComPtr<ID3D11RasterizerState> solidCcwRS;
extern ComPtr<ID3D11RasterizerState> depthRS;

extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCcwRS;
extern ComPtr<ID3D11RasterizerState> postProcessingRS;
extern ComPtr<ID3D11RasterizerState> solidBothRS; // front and back
extern ComPtr<ID3D11RasterizerState> wireBothRS;
extern ComPtr<ID3D11RasterizerState> solidBothCcwRS;
extern ComPtr<ID3D11RasterizerState> wireBothCcwRS;

// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS; // �Ϲ������� �׸���
extern ComPtr<ID3D11DepthStencilState> maskDSS; // ���ٽǹ��ۿ� ǥ��
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // ���ٽ� ǥ�õ� ����

// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11VertexShader> skinnedVS; // basicVS.hlsl�� SKINNED ��ũ��
extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11VertexShader> samplingVS;
extern ComPtr<ID3D11VertexShader> normalVS;
extern ComPtr<ID3D11VertexShader> depthOnlyVS;
extern ComPtr<ID3D11VertexShader> depthOnlySkinnedVS;
extern ComPtr<ID3D11VertexShader> grassVS;
extern ComPtr<ID3D11VertexShader> billboardVS;
extern ComPtr<ID3D11VertexShader> terrainVS;

extern ComPtr<ID3D11HullShader> terrainHS;

extern ComPtr<ID3D11DomainShader> terrainDS;
extern ComPtr<ID3D11DomainShader> terrainDepthDS;

extern ComPtr<ID3D11PixelShader> basicPS;
extern ComPtr<ID3D11PixelShader> billboardPS;
extern ComPtr<ID3D11PixelShader> skyboxPS;
extern ComPtr<ID3D11PixelShader> combinePS;
extern ComPtr<ID3D11PixelShader> bloomDownPS;
extern ComPtr<ID3D11PixelShader> bloomUpPS;
extern ComPtr<ID3D11PixelShader> normalPS;
extern ComPtr<ID3D11PixelShader> depthOnlyPS;
extern ComPtr<ID3D11PixelShader> postEffectsPS;
extern ComPtr<ID3D11PixelShader> volumeSmokePS;
extern ComPtr<ID3D11PixelShader> grassPS;
extern ComPtr<ID3D11PixelShader> oceanPS;
extern ComPtr<ID3D11PixelShader> volumetricFirePS;
extern ComPtr<ID3D11PixelShader> gameExplosionPS;
extern ComPtr<ID3D11PixelShader> terrainPS;

extern ComPtr<ID3D11ComputeShader> editTexureMapCS;

extern ComPtr<ID3D11GeometryShader> normalGS;
extern ComPtr<ID3D11GeometryShader> billboardGS;

// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> skinnedIL;
extern ComPtr<ID3D11InputLayout> samplingIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;
extern ComPtr<ID3D11InputLayout> postProcessingIL;
extern ComPtr<ID3D11InputLayout> grassIL;
extern ComPtr<ID3D11InputLayout> billboardIL;

// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;
extern ComPtr<ID3D11BlendState> accumulateBS;
extern ComPtr<ID3D11BlendState> alphaBS;

// Graphics Pipeline States
extern GraphicsPSO defaultSolidPSO;
extern GraphicsPSO skinnedSolidPSO;
extern GraphicsPSO defaultWirePSO;
extern GraphicsPSO skinnedWirePSO;
extern GraphicsPSO stencilMaskPSO;
extern GraphicsPSO reflectSolidPSO;
extern GraphicsPSO reflectSkinnedSolidPSO;
extern GraphicsPSO reflectWirePSO;
extern GraphicsPSO reflectSkinnedWirePSO;
extern GraphicsPSO mirrorBlendSolidPSO;
extern GraphicsPSO mirrorBlendWirePSO;
extern GraphicsPSO skyboxSolidPSO;
extern GraphicsPSO skyboxWirePSO;
extern GraphicsPSO reflectSkyboxSolidPSO;
extern GraphicsPSO reflectSkyboxWirePSO;
extern GraphicsPSO normalsPSO;
extern GraphicsPSO depthOnlyPSO;
extern GraphicsPSO depthOnlySkinnedPSO;
extern GraphicsPSO postEffectsPSO;
extern GraphicsPSO postProcessingPSO;
extern GraphicsPSO boundingBoxPSO;
extern GraphicsPSO grassSolidPSO;
extern GraphicsPSO grassWirePSO;
extern GraphicsPSO oceanPSO;
extern GraphicsPSO terrainSolidPSO;
extern GraphicsPSO terrainWirePSO;
extern GraphicsPSO terrainDepthPSO;

// ����: �ʱ�ȭ�� ������ �ʿ��� ��쿡�� �ʱ�ȭ
extern GraphicsPSO volumeSmokePSO;

extern ComputePSO editTexturePSO;
void InitCommonStates(ComPtr<ID3D11Device> &device);

// ���������� InitCommonStates()���� ���
void InitSamplers(ComPtr<ID3D11Device> &device);
void InitRasterizerStates(ComPtr<ID3D11Device> &device);
void InitBlendStates(ComPtr<ID3D11Device> &device);
void InitDepthStencilStates(ComPtr<ID3D11Device> &device);
void InitPipelineStates(ComPtr<ID3D11Device> &device);
void InitShaders(ComPtr<ID3D11Device> &device);

// ����: �ʱ�ȭ�� ������ �ʿ��� ��쿡�� �ʱ�ȭ
void InitVolumeShaders(ComPtr<ID3D11Device> &device);

} // namespace Graphics

} // namespace hlab
