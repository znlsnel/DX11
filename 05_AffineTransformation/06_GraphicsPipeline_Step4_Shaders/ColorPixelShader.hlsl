// TODO: �޾ƿ� constant ����

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    // TODO: ���ؽ� ���̴��� �����ֱ� (�ؽ��� ��ǥ �߰�)
};

float4 main(PixelShaderInput input) : SV_TARGET {

    // TODO: �ؽ��� ��ǥ�� �̿��ؼ� �� ����

    // Use the interpolated vertex color
    return float4(input.color, 1.0);
}
