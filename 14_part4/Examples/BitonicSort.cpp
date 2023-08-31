#include "BitonicSort.h"

#include <algorithm>
#include <assert.h>
#include <execution>
#include <random>

namespace hlab {

using namespace std;

void PrintArray(const vector<BitonicSort::Element> &arr) {
    cout << "K: ";
    for (const auto &element : arr)
        cout << element.key << " ";
    cout << endl;

    cout << "V: ";
    for (const auto &element : arr)
        cout << element.value << " ";
    cout << endl;
}

bool Compare(const vector<BitonicSort::Element> &a,
             const vector<BitonicSort::Element> &b) {
    if (a.size() != b.size())
        return false;

    int64_t aSum = 0, bSum = 0;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i].key != b[i].key)
            return false;
        aSum += a[i].value;
        bSum += b[i].value;
    }

    if (aSum != bSum)
        return false;

    return true;
}

void BitonicSort::Initialize(ComPtr<ID3D11Device> &device,
                             const UINT numElements,
                             const wstring shaderFilename) {

    // 2�� �������� Ȯ��
    // https://stackoverflow.com/questions/108318/how-can-i-test-whether-a-number-is-a-power-of-2
    assert(numElements > 0);
    assert((numElements & (numElements - 1)) == 0);

    m_numElements = numElements;

    m_array.Initialize(device, numElements);

    D3D11Utils::CreateComputeShader(device, shaderFilename, m_bitonicSortCS);

    // �ʿ��� ConstBuffer ���� �̸� ����� �α�
    for (uint32_t k = 2; k <= numElements; k *= 2)
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            Consts c;
            c.j = j;
            c.k = k;
            m_constsCpu.push_back(c);
        }
    m_constsGpu.resize(m_constsCpu.size());
    for (size_t i = 0; i < m_constsCpu.size(); i++) {
        D3D11Utils::CreateConstBuffer(device, m_constsCpu[i], m_constsGpu[i]);
    }
}

void BitonicSort::SortGPU(ComPtr<ID3D11Device> &device,
                          ComPtr<ID3D11DeviceContext> &context) {

    size_t constCount = 0;
    for (uint32_t k = 2; k <= m_numElements; k *= 2)
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            context->CSSetConstantBuffers(
                0, 1, m_constsGpu[constCount++].GetAddressOf());
            context->CSSetShader(m_bitonicSortCS.Get(), 0, 0);
            context->CSSetUnorderedAccessViews(0, 1, m_array.GetAddressOfUAV(),
                                               NULL);
            context->Dispatch(UINT(ceil(m_numElements / 1024.0f)), 1, 1);
        }

    // UAV Barrier
    ID3D11ShaderResourceView *nullSRV[2] = {0, 0};
    context->CSSetShaderResources(0, 2, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[2] = {0, 0};
    context->CSSetUnorderedAccessViews(0, 2, nullUAV, NULL);
}

void BitonicSort::SortCPU(vector<Element> &arr) {

    const size_t numElements = arr.size();

    for (uint32_t k = 2; k <= numElements; k *= 2)
        for (uint32_t j = k / 2; j > 0; j /= 2) {

#pragma omp parallel for

            for (int32_t i = 0; i < int32_t(numElements); i++) {
                int32_t l = i ^ j;
                if (l > i) {
                    if (((i & k) == 0) && (arr[i].key > arr[l].key) ||
                        ((i & k) != 0) && (arr[i].key < arr[l].key))
                        std::swap(arr[i], arr[l]);
                }
            }

            // cout << j << endl;
            // PrintArray(arr);
        }
}

void BitonicSort::TestBitonicSort(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context) {

    Timer timer(device);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dc(0, uint32_t(99));

    for (uint32_t numElements = 1024; numElements <= 1024 * pow(2, 10);
         numElements *= 2) {

        cout << "Test Num Elements " << numElements << endl;

        // 1. ���� ������ �غ�
        vector<Element> arr(numElements);
        std::generate(arr.begin(), arr.end(), [&]() -> Element {
            return {dc(gen), dc(gen)};
        });
        vector<Element> arrCopy1(arr); // �纻
        vector<Element> arrCopy2(arr); // �纻

        // 2. std::sort() ���
        {
            timer.Start(context, false);
            std::sort(std::execution::par, arrCopy1.begin(), arrCopy1.end(),
                      [](const Element &a, const Element &b) -> bool {
                          return a.key < b.key;
                      });
            timer.End(context);
        }

        // 3. Wikipedia ����
        {
            timer.Start(context, false);
            SortCPU(arr);
            timer.End(context);
            cout << (Compare(arr, arrCopy1) ? "OK" : "NOT OK") << endl;
        }

        // 4. GPU ����
        {
            BitonicSort sort(device, UINT(arrCopy2.size()),
                             L"Ex1408_BitonicSortCS.hlsl");
            sort.m_array.Upload(context, arrCopy2);

            timer.Start(context, true);
            sort.SortGPU(device, context);
            timer.End(context);

            sort.m_array.Download(context, arrCopy2);
            cout << (Compare(arr, arrCopy2) ? "OK" : "NOT OK") << endl;

            // PrintArray(arrCopy2);
        }

        // �� PC ���� �޸� ���縦 �����ϰ� std::sort()�� ����մϴ�.
        // sort() ��ü�� ������ �ʴ��� CPU-GPU ���縦 ���� �� �ִٴ� ����
        // �����Դϴ�.

        // �߰� ����ȭ ����: MS Sample������ j ������ ���̴� ������ �Ű���ϴ�.
        // https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/Bitonic32InnerSortCS.hlsl
    }

    exit(0);
}

} // namespace hlab