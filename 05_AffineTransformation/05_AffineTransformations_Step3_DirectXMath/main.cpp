// SimpleMath
// vcpkg install directxtk:x64-windows
#include <DirectXMath.h>
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <iostream>

using namespace DirectX;
using namespace std;

ostream &operator<<(ostream &os, XMFLOAT4 m) {
    os << m.x << "\t" << m.y << "\t" << m.z << endl;
    return os;
}

ostream &operator<<(ostream &os, XMFLOAT4X4 m) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            os << m.m[i][j] << "\t";
        }
        os << endl;
    }
    return os;
}

int main() {

    // DirectXMath programming guide 참고
    // https://learn.microsoft.com/en-us/windows/win32/dxmath/ovw-xnamath-progguide

    // X64에서는 기본
    // Enable Enhanced Instruction Set -> SSE2
    // https://github.com/weidai11/cryptopp/pull/446

    if (!XMVerifyCPUSupport()) {
        cout << "directx math not supported" << endl;
        return -1;
    }

    // XMVECTOR
    // SIMD 가속 사용

    // 클래스 멤버로는 XMFLOAT2, 3, 4 사용
    // 연산하기 전에 XMVECTOR로 변환
    // Type Usage Guidelines
    // https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started#type-usage-guidelines

    // 요약
    // 1. XMVECTOR: 지역 또는 전역 변수
    // 2. XMFLOAT2, 3, 4: 클래스 멤버
    // 3. XMStoreFloat2, 3, 4: XMVECTOR -> XMFLOAT2, 3, 4
    // 4. XMVECTOR로 연산
    // 5. 결과를 다시 XMFLOATN으로 저장

    // DirectXMath를 이용해서 벡터의 길이를 구하는 경우
    XMFLOAT4 xfloat4 = {1.0f, 2.0f, 3.0f, 1.0f};
    XMVECTOR xvector = XMLoadFloat4(&xfloat4);
    xvector = XMVector3Length(xvector); // sqrt(1*1 + 2*2 + 3*3), 함수 이름이
                                        // XMVector 숫자3 Length() 입니다.

    float length;
    XMStoreFloat(&length, xvector);

    // 행렬을 이용한 변환 예시

    XMMATRIX tr = XMMatrixTranslation(1.0f, 2.0f, 3.0f);

    XMFLOAT4X4 tr4;
    XMStoreFloat4x4(&tr4, tr);

    cout << tr4 << endl;
    // 1 0 0 0
    // 0 1 0 0
    // 0 0 1 0
    // 1 2 3 1

    XMFLOAT4 myPoint = {4.0f, 5.0f, 6.0f, 1.0f};
    XMFLOAT4 myVector = {4.0f, 5.0f, 6.0f, 0.0f};

    XMVECTOR temp = XMLoadFloat4(&myPoint);

    XMVECTOR result = XMVector3TransformCoord(temp, tr); // XMVector4Transform

    XMStoreFloat4(&myPoint, result);
    cout << myPoint << endl;
    // 5       7       9

    temp = XMLoadFloat4(&myVector);

    result = XMVector3TransformNormal(temp, tr); // XMVector4Transform

    XMStoreFloat4(&myVector, result);
    cout << myVector << endl;
    // 4       5       6

    // SimpleMath
    // vcpkg install directxtk:x64-windows
    // #include <directxtk/SimpleMath.h>
    {
        using namespace DirectX::SimpleMath;

        Matrix tr = Matrix::CreateTranslation(Vector3(1.0f, 2.0f, 3.0f));

        // 메모리에 어떤 순서로 저장되는지 확인
        // cout << "Transtion Matrix" << endl;
        // for (int i = 0; i < 16; i++) {
        //     cout << ((float *)&tr)[i] << " ";
        // }
        // cout << endl;
        // 출력결과: 1 0 0 0 0 1 0 0 0 0 1 0 1 2 3 1

        Vector4 myPoint(4.0f, 5.0f, 6.0f, 1.0f);
        Vector4 myVector(4.0f, 5.0f, 6.0f, 0.0f);

        myPoint = Vector4::Transform(myPoint, tr);
        myVector = Vector4::Transform(myVector, tr);

        cout << myPoint << endl;
        // 5       7       9

        cout << myVector << endl;
        // 4       5       6

        cout << tr << endl;
        // 1 0 0 0
        // 0 1 0 0
        // 0 0 1 0
        // 1 2 3 1

        tr.Translation(Vector3(0.0f));
        cout << tr << endl;

        // 1 0 0 0
        // 0 1 0 0
        // 0 0 1 0
        // 0 0 0 1

        // Matrix::CreateScale();
        // Matrix::CreateRotationY();
        // Matrix::CreateRotationX();
        // Matrix::CreateTranslation();
        // M.Translation(Vector3(0.0f));
        // M.Invert().Transpose();
        // Vector4::Transform(point, constants.modelMatrix);
        // n.Normalize();
    }

    return 0;
}
