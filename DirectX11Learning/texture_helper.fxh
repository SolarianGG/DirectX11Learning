SamplerState samAntisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerState samNear
{
    Filter = MIN_MAG_MIP_POINT;

    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = WRAP;
    AddressV = WRAP;
};
