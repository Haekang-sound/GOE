
cbuffer CB : register(b0)
{
    matrix mvp;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;
    result.position = mul(position, mvp);
    result.color = color;
    return result;


}
