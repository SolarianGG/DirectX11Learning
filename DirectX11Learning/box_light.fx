#include "light_helper.fxh"

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldProjectView;
    float4x4 gWorldInverseTranspose;
    Material gMat;
};

cbuffer cbPerFrame
{
    PointLight gPointLight;
    DirectionalLight gDirectionalLight;
    float3 gEyePosW;
};

struct VertexIn
{
    float3 posL : POSITION;
    float3 normL : NORMAL;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 posW : POSITION;
    float3 normW : NORMAL;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.posH = mul(float4(vIn.posL, 1.0f), gWorldProjectView);
    vOut.posW = mul(float4(vIn.posL, 1.0f), gWorld).xyz;
    vOut.normW = mul(vIn.normL, (float3x3) gWorldInverseTranspose);
    return vOut;
}

float4 PS(VertexOut vOut) : SV_Target
{
    vOut.normW = normalize(vOut.normW);
    float3 toEyeW = normalize(gEyePosW - vOut.posW);
    
    float4 ambient = float4(0.f, 0.f, 0.f, 0.f);
    float4 diffuse = float4(0.f, 0.f, 0.f, 0.f);
    float4 spec = float4(0.f, 0.f, 0.f, 0.f);
    
    float4 A, B, C;
    
    ComputeDirectionalLight(gMat, gDirectionalLight, vOut.normW, toEyeW, A, B, C);
    ambient += A;
    diffuse += B;
    spec += C;
    
    ComputePointLight(gMat, gPointLight, vOut.posW, vOut.normW, toEyeW, A, B, C);
    ambient += A;
    diffuse += B;
    spec += C;
    
    float4 litColor = ambient + diffuse + spec;
    litColor.a = gMat.Diffuse.a;
    return litColor;
}


technique11 LightTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}