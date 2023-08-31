Texture2D<float4> matTex : register(t0); // (width, height) = (numCols/4, numRows/4)
Texture2D<float4> vecTex : register(t1); // (width, height) = (numCols/4, 1)
RWTexture2D<float> outputTex : register(u0); // (width, height) = (numRows/4, 1)

// ��� ������ ����(numRows / 4)�� ���� ���� ������� ������ ���

// ������� ���� (1, 1, 1)�ε� �׽�Ʈ
[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    matTex.GetDimensions(width, height);

    uint r = dtID.x;
    float sum = 0.0;
    for (uint i = 0; i < width; i++)
    {
        // TODO:
    }

    // outputTex[dtID.xy].r = sum; // ��� ���� ���ϱ�

    outputTex[dtID.xy].r += sum; // �ݺ� ���� �׽�Ʈ
}
