// TODO: �޾ƿ� constant ����

cbuffer PixelShaderConstantBuffer : register(b0) { float xSplit; }

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD0;

    // TODO: ���ؽ� ���̴��� �����ֱ� (�ؽ��� ��ǥ �߰�)
};

float4 main(PixelShaderInput input) : SV_TARGET {

    // TODO: �ؽ��� ��ǥ�� �̿��ؼ� �� ����

    // Use the interpolated vertex color

    return input.texcoord.x > xSplit ? float4(0.0, 0.0, 1.0, 1.0)
                                     : float4(1.0, 0.0, 0.0, 1.0);
}
