struct VertexIn
{
    float4 pos : POSITION;
};

struct VertexOut
{
    float4 pos : POSITION;
};

VertexIn main(VertexIn vin)
{
    VertexOut vout;
	
    vout.pos = vin.pos;

    return vin;
}
