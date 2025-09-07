// shader_vs.hlsl

cbuffer CB : register(b0)
{
    matrix mvp;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0; // 텍스처 좌표를 넘겨주기 위한 변수 추가
};

// 정점 데이터에서 uv 좌표(TEXCOORD)를 입력받도록 수정
PSInput Main(float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = mul(position, mvp);
    result.color = color;
    result.uv = uv; // 입력받은 uv를 그대로 결과에 넘겨줌

    return result;
}