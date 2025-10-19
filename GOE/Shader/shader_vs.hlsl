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
    float2 uv : TEXCOORD0; 
    float3 normal : NORMAL;
};



// 셰이더 함수의 매개변수와 IA에서 전달되는 데이터 구조체 정의는 동일해야 합니다.
PSInput Main(
float4 position : POSITION,
float4 color : COLOR,
float2 uv : TEXCOORD,
float3 normal : NORMAL,
uint4 boneIndices : BLENDINDICES,
float4 boneWeights : BLENDWEIGHT)
{
    PSInput result;

    // ② 스키닝 매트릭스 계산
    matrix skinMatrix = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        uint idx = boneIndices[i];
        float w = boneWeights[i];

        // boneOffset × boneMatrix 조합
        //matrix finalMat = mul(boneOffsetMatrices[idx], boneMatrices[idx]);
        matrix finalMat; // = mul(boneOffsetMatrices[idx], boneMatrices[idx]);
        finalMat[0] = float4(1, 0, 0, 0);
        finalMat[1] = float4(0, 1, 0, 0);
        finalMat[2] = float4(0, 0, 1, 0);
        finalMat[3] = float4(0, 0, 0, 1);
        
        skinMatrix = finalMat;// * w;
    }

    // ③ 정점 위치 스키닝 적용
    float4 skinnedPos = mul(position, skinMatrix);

    // ④ 노멀도 스키닝 적용
    float3 skinnedNormal = mul(normal, (float3x3) skinMatrix);
    skinnedNormal = normalize(skinnedNormal);

    //// ⑤ 월드 / 뷰 / 투영 변환
    //float4 worldPos = mul(skinnedPos, world);
    //result.position = mul(worldPos, viewProjection);
    
    
    result.position = mul(position, mul(world, viewProjection));
    result.color = color;
    result.uv = uv; // 입력받은 uv를 그대로 결과에 넘겨줌
    result.normal = normalize(mul(normal, (float3x3) world));
    
    return result;
}