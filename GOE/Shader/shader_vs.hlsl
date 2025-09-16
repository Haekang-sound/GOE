cbuffer CB : register(b0)
{
    matrix world;
    matrix viewProjection;
    float3 cameraPosition; // 카메라 위치
    float padding; // 16바이트 정렬을 위한 패딩
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0; // 텍스처 좌표를 넘겨주기 위한 변수 추가
    float3 normal : NORMAL; // 법선 벡터를 넘겨주기 위한 변수 추가
};

// 정점 데이터에서 uv 좌표(TEXCOORD)를 입력받도록 수정
PSInput Main(float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD, float3 normal : NORMAL)
{
    PSInput result;

    result.position = mul(position, mul(world, viewProjection));
    result.color = color;
    result.uv = uv; // 입력받은 uv를 그대로 결과에 넘겨줌
    result.normal = normalize(mul(normal, (float3x3) world));
    
    return result;
}