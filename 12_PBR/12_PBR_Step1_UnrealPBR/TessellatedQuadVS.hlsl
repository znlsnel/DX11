struct VertexIn
{
    float4 pos : POSITION;
};

struct VertexOut
{
    float4 pos : POSITION;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
	
    vout.pos = vin.pos;

    return vout;
}
