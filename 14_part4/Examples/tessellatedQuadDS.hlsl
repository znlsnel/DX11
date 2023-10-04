
struct HullOut
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; // Volume casting 시작점
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; // Volume casting 시작점
};
struct PatchConstOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
    
};

[domain("quad")]
PixelShaderInput main(PatchConstOutput patchConst,
             float2 uv : SV_DomainLocation,
             const OutputPatch<HullOut, 4> quad)
{
    PixelShaderInput result;
 
    result.normalWorld = quad[0].normalWorld;
    result.posModel = quad[0].posModel;
    result.posProj = quad[0].posProj;
    result.posWorld = quad[0].posWorld;
    result.tangentWorld = quad[0].tangentWorld;
    result.texcoord = quad[0].texcoord;
    
    return result;
    
    
	//// Bilinear interpolation.
 //   float3 v1 = lerp(quad[0].posWorld, quad[1].posWorld, uv.x);
 //   float3 v2 = lerp(quad[2].posWorld, quad[3].posWorld, uv.x);
 //   float3 p = lerp(v1, v2, uv.y);
 //   result.posWorld = float4(p, 1.0);
    
 //  v1 = lerp(quad[0].normalWorld, quad[1].normalWorld, uv.x);
 //   v2 = lerp(quad[2].normalWorld, quad[3].normalWorld, uv.x);
 //   p = lerp(v1, v2, uv.y);
 //   result.normalWorld = p;
    
 //    v1 = lerp(quad[0].posModel, quad[1].posModel, uv.x);
 //    v2 = lerp(quad[2].posModel, quad[3].posModel, uv.x);
 //    p = lerp(v1, v2, uv.y);
 //   result.posModel = p;
    
 //    v1 = lerp(quad[0].posProj.xyz, quad[1].posProj.xyz, uv.x);
 //   v2 = lerp(quad[2].posProj.xyz, quad[3].posProj.xyz, uv.x);
 //   p = lerp(v1, v2, uv.y);
 //   result.posProj = float4(p, 1.0);
    
 //   v1 = lerp(quad[0].tangentWorld, quad[1].tangentWorld, uv.x);
 //   v2 = lerp(quad[2].tangentWorld, quad[3].tangentWorld, uv.x);
 //    p = lerp(v1, v2, uv.y);
 //   result.tangentWorld = p;
    
 //   float2 v3 = lerp(quad[0].texcoord, quad[1].texcoord, uv.x);
 //   float2 v4 = lerp(quad[2].texcoord, quad[3].texcoord, uv.x);
 //   float2 p2 = lerp(v1, v2, uv.y);
 //   result.texcoord = p2;
    

    
 //   return result;
}
