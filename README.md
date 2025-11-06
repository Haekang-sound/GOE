
https://github.com/user-attachments/assets/0150aee0-0b1d-439d-b397-fb3914b84a31

# 자체 엔진 프로젝트 : GOE(괴)

실행환경 : 64x, (Debug/Release)\
실행파일 : GOE.sln

## 1. 프로젝트 개요
  **GOE (Game Oriented Engine)**는 DirectX 12를 기반으로 한 3D 게임 엔진입니다. 최신 C++(17) 표준을 사용하여 객체 지향 및 데이터 지향 설계를 결합하였으며, 모듈성과 확장성을 고려하여 개발되었습니다. ECS(Entity-Component-System) 아키텍처를 채택하여 데이터와 로직을 분리하고, 유연한 게임 오브젝트 생성을 지원합니다.

## 2. 기술설명
### 2.1 모듈식 아키텍처 (Modular Architecture)
엔진의 기능을 역할에 따라 명확하게 분리된 여러 개의 프로젝트(정적 라이브러리)로 구성하여 높은 수준의 모듈성을 확보했습니다. 이는 각 모듈의 독립적인 개발 및 테스트를 용이하게 하고, 유지보수성을 크게 향상시킵니다.
* GOE: 최종 실행 파일을 생성하는 메인 애플리케이션 프로젝트입니다.
* GOE_Core: WindowAPI, 범용구조체/인터페이스, 엔진 전용 수학자료등 여러 라이브러리에서 사용되는 범용적인 자료들을 관리합니다.
* GOE_Engine: ECS 패턴을 구현하고, 씬(Scene)을 관리하며 각 시스템을 총괄하는 엔진의 핵심 로직을 포함합니다.
* GOE_Render: Directx관련 자료형/리소스, DirectXMath수학구조체등을 관리하고 D3D12 API를 직접 제어하여 실제 화면 출력을 담당합니다.
* GOE_AssetLoader: Assimp 라이브러리를 활용하여 FBX와 같은 3D 모델 파일을 로드하고, 엔진에서 사용할 수 있는 데이터로 변환합니다.
* GOE_Editor: ImGui를 통합하여 디버깅 및 에디터 기능을 제공합니다.

### 2.2 ECS (Entity-Component-System) 디자인 패턴 적용
현대 게임 엔진의 표준 아키텍처인 ECS 패턴을 도입하여 데이터와 로직을 분리했습니다.
* Entity: 게임 세계에 존재하는 모든 오브젝트를 나타내는 고유 ID입니다.
* Component: Transform, MeshRenderer, MovementUnit 등 오브젝트가 가질 수 있는 데이터 조각들입니다. 각 컴포넌트는 순수 데이터 컨테이너 역할을 합니다.
* System: RenderSystem, MovementSystem, TransfromSystem 등 특정 컴포넌트들을 처리하여 실제 로직을 수행합니다. 이를 통해 코드의 재사용성과 유연성을 극대화했습니다.
* Manager: EntityManager, ComponentManager<T>를 통해 엔티티와 컴포넌트를 효율적으로 생성, 조회 및 관리합니다.

```c
// Scene.cpp의 Script 함수 예시: ECS 패턴을 활용한 엔티티 생성
void Scene::Script()
{
    // "쿠라몬"이라는 이름의 엔티티를 생성합니다.
    m_entityManager.get()->CreateEntity("쿠라몬");

    // 방금 생성된 엔티티에 Transform 컴포넌트를 추가하고 초기화합니다.
    m_transformManager.get()->AddComponent(
        m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
        m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
    GOE::Matrix4x4 tm = GOE::Matrix4x4::Identity();
    m_transformManager.get()->GetCurrentComponent().SetLocalTM(tm);
    m_transformManager.get()->GetCurrentComponent().SetScaleTM({ 100.f, 100.f, 100.f });

    // MeshRenderer 컴포넌트를 추가하여 렌더링할 메쉬 정보를 지정합니다.
    m_meshRendererManager.get()->AddComponent(
        m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
        m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
    size_t meshPath = GOE::FileManager::GetHash("chr629_0");
    m_meshRendererManager.get()->GetCurrentComponent().SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
    // ...
}
```

### 2.3 DirectX 12 기반의 렌더링 파이프라인
현재 D3D12의 핵심 기능을 사용해서 텍스쳐를 입힌 모델을 랜더하였습니다. 단기목표는 스키닝구현, 이후로도 계속 개발을 이어갈 계획입니다.
* 명령 기반 렌더링: CommandQueue, CommandAllocator, CommandList를 사용하여 렌더링을 수행합니다.
* 리소스 관리 및 동기화: Descriptor Heap, Resource Barrier를 통해 GPU 리소스의 상태를 명시적으로 관리하며, Fence를 이용한 CPU-GPU 동기화를 구현하여 안정적인 렌더링을 보장합니다.
* 셰이더 관리: 최신 셰이더 컴파일러인 DXC를 사용하여 HLSL 셰이더를 컴파일하며, Root Signature와 PSO(PipelineState Object)를 통해 셰이더 리소스를 바인딩합니다.
* 디버깅 및 프로파일링: PIX for Windows 와의 연동 코드를 포함하여 D3D12 렌더링 디버깅 및 성능 분석 능력을 갖추었습니다.

### 2.3 UI
개발 편의성을 고려한 디버깅 시스템 ImGui를 엔진에 통합하여 런타임 중에 엔진의 주요 상태(카메라 위치, 객체 정보 등)를 확인하고 수정할 수 있는 디버깅 UI를 구축했습니다.
* DebugManager를 싱글턴으로 구현하여 엔진의 어느 곳에서든 디버그 정보를 쉽게 출력할 수 있도록 설계했습니다.

### 2.4 스키닝
모델의 뼈대(Bone) 움직임에 따라 정점(Vertex)이 변형되는 스키닝 애니메이션을 구현하였습니다. 전체 프로세스는 데이터 로드, 본 애니메이션 (CPU), **스키닝 (GPU)**의 세 단계로 구성됩니다.

### 2.4.1 애니메이션 데이터 로드 (AssetLoader)
Assimp 라이브러리를 활용하여 FBX 파일에서 애니메이션 데이터를 추출합니다.
* Assimp의 aiAnimation, aiNodeAnim, aiKeyFrame 데이터를 파싱하여, 엔진 고유의 Animation, BoneAnimation 자료구조에 변환하여 저장합니다.<br>

### 2.4.2 본 애니메이션 (CPU)
매 프레임 dTime을 기준으로, CPU에서 각 뼈의 변환 행렬을 계산합니다.  
* 키프레임 보간 (Interpolation):
dTime을 누적하여 현재 애니메이션의 Tick을 계산하고, FindKeyIndex 함수로 현재 시간대에 해당하는 키프레임 인덱스(A, B)를 탐색합니다.  
두 키프레임 사이의 비율(factor)을 계산합니다.  
위치(Position)와 스케일(Scale)은 **선형 보간 (Lerp)**을, 회전(Rotation)은 **구면 선형 보간 (Slerp)**을 사용하여 최종 Local 변환 행렬을 계산합니다.  

* 계층구조 업데이트 (Hierarchy Update):
계산된 Local 행렬을 루트 노드부터 자식 노드까지 순회하며 부모 노드의 World 행렬과 곱합니다. (World = Local * ParentWorld)  
이 과정을 통해 모든 뼈의 최종 World 변환 행렬(boneMatrices)을 완성합니다.  

* 상수 버퍼 전송:
계산된 최종 boneMatrices 배열과, 모델 로드 시 미리 저장해둔 역 바인드 포즈 행렬(boneOffsets) 배열을 GPU의 상수 버퍼(Constant Buffer)로 전송합니다.<br>

### 2.4.3 스키닝 애니메이션 (GPU)
최종적인 정점 변환은 버텍스 셰이더(Vertex Shader)에서 GPU가 직접 수행합니다.
* 정점 데이터 확장: 정점 구조체(Vertex)에 boneIndices[4] (영향을 주는 뼈 ID)와 boneWeights[4] (뼈의 영향도)를 추가합니다.
* 가중치 정규화: Assimp에서 로드한 가중치의 총합이 1.0이 되도록 CPU에서 정규화(Normalization) 과정을 거칩니다.
* HLSL 셰이더 연산: 버텍스 셰이더는 각 정점마다 최대 4개의 뼈에 대해 스키닝 연산을 수행합니다. 핵심 로직은 다음과 같습니다.
  
```HLSL
// cbuffer (b1) : boneMatrices (애니메이션이 적용된 World 행렬)
// cbuffer (b2) : boneOffsets (역 바인드 포즈 행렬)

PSInput Main(...,
             uint4 boneIndices : BONEINDICES,
             float4 boneWeights : BONEWEIGHTS)
{
    PSInput result;
    matrix skinM = (matrix)0; // 최종 스키닝 행렬
    float4 finalPos = (float4)0;
    
    // 4개의 뼈에 대해 가중치를 적용한 변환 행렬을 계산
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        uint idx = boneIndices[i];
        float w = boneWeights[i];
        
        // 역 바인드 포즈로 정점을 원점으로 돌리고,
        // 현재 애니메이션 뼈 위치로 이동시킨 후, 가중치를 곱함
        matrix offset = boneOffsets[idx];
        matrix boneM = boneMatrices[idx];
        
        skinM += w * mul(boneM, offset);
    }
    
    // 최종 스키닝 행렬을 정점 위치(position)와 노멀(normal)에 적용
    finalPos = mul(skinM, position);
    finalPos = mul(world, finalPos);
    result.position = mul(viewProjection, finalPos);

    float3 skinnedNormal = mul((float3x3)skinM, normal);
    skinnedNormal = normalize(skinnedNormal);
    result.normal = normalize(mul((float3x3)world, skinnedNormal));

    return result;
}  
``` 
