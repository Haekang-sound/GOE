
cbuffer CB : register(b0)
{
    matrix world;
    matrix viewProjection;
    float3 cameraPosition; // 카메라 위치
    float padding; // 16바이트 정렬을 위한 패딩
};

cbuffer bone : register(b1)
{
    matrix boneMatrices[128]; // 애니메이션 적용된 최종 행렬
};

cbuffer offset : register(b2)
{
    matrix boneOffsets[128]; // 역바인드 포즈(Inverse Bind Pose) 행렬
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

    matrix skinM = (matrix) 0;
    float4 finalPos = (float4) 0;
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        uint idx = boneIndices[i];
        float w = boneWeights[i];
        matrix offset = boneOffsets[idx];
        matrix boneM = boneMatrices[idx];
        
        skinM += w * mul(boneM, offset);
    }
    
    finalPos = mul(skinM, position);
    finalPos = mul(world, finalPos);
    result.position = mul(viewProjection, finalPos);

    // ④ 노멀도 스키닝 적용 (모델 공간)
    float3 skinnedNormal = mul((float3x3) skinM, normal);
    skinnedNormal = normalize(skinnedNormal);
    // [수정 3] 스키닝이 적용된 노멀(skinnedNormal)을 월드 변환합니다.
    result.normal = normalize(mul((float3x3) world, skinnedNormal));
    
    // ⑤ 월드 / 뷰 / 투영 변환
    result.color = color;
    result.uv = uv;

    
    return result;
}