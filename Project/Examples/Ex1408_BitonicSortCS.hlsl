struct Element
{
    uint key;
    uint value;
};

cbuffer MyBuffer : register(b0)
{
    // https://en.wikipedia.org/wiki/Bitonic_sorter Example Code
    uint k;
    uint j;
}

RWStructuredBuffer<Element> arr : register(u0);

[numthreads(1024, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    // ��Ʈ: (value ����) key�� �����Ͻø� �˴ϴ�.
    uint i = dtID.x;
        uint l = i ^ j; 
        if (l > i)
        {
            if (((i & k) == 0) && (arr[i].key > arr[l].key) ||
                        ((i & k) != 0) && (arr[i].key < arr[l].key))
            {
                Element temp = arr[i];
                arr[i] = arr[l];
                arr[l] = temp;
               
            }
        }
    
    
}