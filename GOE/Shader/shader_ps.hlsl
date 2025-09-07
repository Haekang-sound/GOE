// shader_ps.hlsl

// C++에서 설정한 텍스처 리소스(t0)와 샘플러(s0)
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0; // 버텍스 셰이더에서 넘겨준 uv 좌표
};

float4 Main(PSInput input) : SV_TARGET
{
    // 텍스처와 샘플러를 사용해 uv 좌표에 해당하는 색상을 가져옴
    float4 textureColor = txDiffuse.Sample(samLinear, input.uv);
    
    return textureColor * input.color;
}