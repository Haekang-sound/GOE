// C++에서 설정한 텍스처 리소스(t0)와 샘플러(s0)
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0; // 텍스처 좌표를 넘겨주기 위한 변수 추가
    float3 normal : NORMAL; // 법선 벡터를 넘겨주기 위한 변수 추가
};

float4 Main(PSInput input) : SV_TARGET
{
    // 텍스처와 샘플러를 사용해 uv 좌표에 해당하는 색상을 가져옴
    float4 textureColor = txDiffuse.Sample(samLinear, input.uv);
    
    float3 lightDir = normalize(float3(0.0, 0.0, -1.0)); // 광원 방향 벡터 (예: 위쪽과 약간 앞쪽)
    
    float3 normal = normalize(input.normal);

    // 4. 법선 벡터와 광원 방향 벡터를 '내적'하여 빛의 세기를 계산합니다.
    //    결과값은 두 벡터 사이 각도의 코사인 값입니다.
    float diffuseFactor = dot(normal, lightDir);
     
    // 2. 조명 계산은 색상(RGB)에만 적용합니다.
    float3 finalColor = textureColor.rgb * input.color.rgb * diffuseFactor;
    
    //  3. 알파 값은 원래 텍스처의 알파 값을 그대로 사용합니다.
    return float4(finalColor, textureColor.a);
}