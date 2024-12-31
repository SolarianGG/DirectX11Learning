
#include "light_helper.fxh"

cbuffer cbPerFrame
{
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
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
    vOut.posW = mul(float4(vIn.posL, 1.f), gWorld).xyz;
    vOut.normW = mul(vIn.normL, (float3x3) gWorldInvTranspose); // ?
    vOut.posH = mul(float4(vIn.posL, 1.f), gWorldViewProj);
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
	   
    float4 litColor = ambient + diffuse + spec;

	// Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a;

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