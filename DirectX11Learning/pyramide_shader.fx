cbuffer cbPerObject
{
    float4x4 gWorldMatrix;
};

cbuffer cbPerFrame
{
    float gTime;
};

struct VertexIn
{
    float3 iPos : POSITION;
    float4 iCol : COLOR;
};

struct VertexOut
{
    float4 oPos : SV_Position;
    float4 oColor : COLOR;
};

float4x4 GetRotationMatrixX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return float4x4(
        1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}

float4x4 GetRotationMatrixY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return float4x4(
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1
    );
}

VertexOut VS(VertexIn v)
{
    VertexOut vOut;
    float4x4 rotationMatrixX = GetRotationMatrixX(gTime);
    float4x4 rotationMatrixY = GetRotationMatrixY(gTime);
    float4x4 finalRotationMatrix = mul(rotationMatrixY, rotationMatrixX);
    float4 rotatedPos = mul(float4(v.iPos, 1.f), finalRotationMatrix);
    vOut.oPos = mul(rotatedPos, gWorldMatrix);
    vOut.oColor = v.iCol;
    return vOut;
}

float4 PS(VertexOut v) : SV_Target
{
    return v.oColor * abs(sin(gTime));
}

RasterizerState SolidRS
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

technique11 PyramideTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetRasterizerState(SolidRS);

    }
}