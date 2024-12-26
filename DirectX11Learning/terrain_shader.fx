cbuffer cbPerObject
{
    float4x4 gWorldProjectView;
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
    vOut.oPosH = mul(float4(v.iPos, 1.f), gWorldProjectView);
    vOut.oColor = v.iColor;
    return vOut;
}

float4 PS(VertexOut v) : SV_Target
{
    return v.oColor;
}

RasterizerState SolidRS
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};


RasterizerState WireFrameRS
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockwise = false;
};

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        
        SetRasterizerState(WireFrameRS);

    }
}