#include "light_helper.fxh"
#include "texture_helper.fxh"

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldProjectView;
    float4x4 gWorldInverseTranspose;
    float4x4 gTexTransform;
    Material gMaterial;
};

cbuffer cbPerFrame
{
    PointLight gPointLight;
    DirectionalLight gDirLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
};

Texture2D gDiffuseMap;

struct VertexIn
{
    float3 posL : POSITION;
    float3 normL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 posW : POSITION;
    float3 normW : NORMAL;
    float2 tex : TEXCOORD;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.posH = mul(float4(vIn.posL, 1.0f), gWorldProjectView);
    vOut.posW = mul(float4(vIn.posL, 1.0f), gWorld).xyz;
    vOut.normW = mul(vIn.normL, (float3x3) gWorldInverseTranspose);
    vOut.tex = mul(float4(vIn.tex, 0.0f, 1.0f), gTexTransform).xy;
    return vOut;
}

float4 PS(VertexOut vOut) : SV_Target
{
    vOut.normW = normalize(vOut.normW);
    
    float3 toEyeW = normalize(gEyePosW - vOut.posW);
    
    float4 ambient = float4(0.f, 0.f, 0.f, 0.f);
    float4 diffuse = float4(0.f, 0.f, 0.f, 0.f);
    float4 spec = float4(0.f, 0.f, 0.f, 0.f);
    
    float4 A, D, S;
    
    float4 texColor = gDiffuseMap.Sample(samAntisotropic, vOut.tex);

    ComputeDirectionalLight(gMaterial, gDirLight, vOut.normW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputePointLight(gMaterial, gPointLight, vOut.posW, vOut.normW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeSpotLight(gMaterial, gSpotLight, vOut.posW, vOut.normW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
	   
    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}


technique11 TextLightTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}