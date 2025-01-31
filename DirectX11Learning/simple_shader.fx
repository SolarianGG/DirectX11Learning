cbuffer cbPerObject
{
    float4x4 gWorldProjectView;
    float time;
};

struct VertexIn
{
    float3 iPos : POSITION;
    float4 iColor : COLOR;
};

struct VertexOut
{
    float4 oPosH : SV_Position;
    float4 oColor : COLOR;
};


VertexOut VS(VertexIn v)
{
    VertexOut vOut;
    v.iPos.xy += 0.5f * sin(v.iPos.x) * sin(3.0f * time);
    v.iPos.z *= 0.6f + 0.4f * sin(2.0f * time);
    vOut.oPosH = mul(float4(v.iPos, 1.f), gWorldProjectView);
    vOut.oColor = v.iColor;
    return vOut;
}

float4 PS(VertexOut v) : SV_Target
{
    v.oColor.z += abs(cos(time));
    return v.oColor;
}
/*
RasterizerState SolidRS
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};*/

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        
       // SetRasterizerState(SolidRS);

    }
}