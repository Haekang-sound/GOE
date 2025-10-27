cbuffer CB : register(b0)
{
    row_major matrix world;
    row_major matrix viewProjection;
    float3 cameraPosition; // 카메라 위치
    float padding; // 16바이트 정렬을 위한 패딩
};

cbuffer bone : register(b1)
{
    row_major matrix boneMatrices[128]; // 애니메이션 적용된 최종 행렬
};

cbuffer offset : register(b2)
{
    row_major matrix boneOffsets[128]; // 역바인드 포즈(Inverse Bind Pose) 행렬
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

// 셰이더 함수의 매개변수와 IA에서 전달되는 데이터 구조체 정의는 동일해야 합니다.
PSInput Main(
float4 position : POSITION,
float4 color : COLOR,
float2 uv : TEXCOORD,
float3 normal : NORMAL,
uint4 boneIndices : BONEINDICES,
float4 boneWeights : BONEWEIGHTS)
{
    PSInput result;

    matrix offsetW = (matrix)0;
    matrix matrixW = (matrix) 0;
    
    matrix skinM= (matrix) 0;
    
     [unroll]
    for (int i = 0; i < 4; ++i)
    {
        uint idx = boneIndices[i];
        float w = boneWeights[i];
        skinM += w * mul(boneOffsets[idx], boneMatrices[idx]);
    }
    
    float4 offsetPose = (float4)0;
    float4 worldPos = (float4) 0;
    float4 finalPos = (float4) 0;

    finalPos = mul(position, skinM);
    finalPos = mul(finalPos, world);

    // ④ 노멀도 스키닝 적용 (모델 공간)
    float3 skinnedNormal = mul(normal, (float3x3) skinM);
    skinnedNormal = normalize(skinnedNormal);

    // ⑤ 월드 / 뷰 / 투영 변환
    result.position = mul(finalPos, viewProjection);
    result.color = color;
    result.uv = uv;

    // [수정 3] 스키닝이 적용된 노멀(skinnedNormal)을 월드 변환합니다.
    result.normal = normalize(mul(skinnedNormal, (float3x3) world)); 
    return result;
}