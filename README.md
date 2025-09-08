# GOE

자체 엔진 포트폴리오: GOE(괴)

## 1. 프로젝트 개요
  **GOE (Game Oriented Engine)**는 DirectX 12를 기반으로 한 3D 게임 엔진입니다. 최신 C++(17) 표준을 사용하여 객체 지향 및 데이터 지향 설계를 결합하였으며, 모듈성과 확장성을 고려하여 개발되었습니다. ECS(Entity-Component-System) 아키텍처를 채택하여 데이터와 로직을 분리하고, 유연한 게임 오브젝트 생성을 지원합니다.

## 2. 어필 포인트 및 기술적 강점
  ### 2.1 모듈식 아키텍처 (Modular Architecture)
  엔진의 기능을 역할에 따라 명확하게 분리된 여러 개의 프로젝트(정적 라이브러리)로 구성하여 높은 수준의 모듈성을 확보했습니다. 이는 각 모듈의 독립적인 개발 및 테스트를 용이하게 하고, 유지보수성을 크게 향상시킵니다.
* GOE: 최종 실행 파일을 생성하는 메인 애플리케이션 프로젝트입니다.
* GOE_Core: WindowAPI, 범용구조체/인터페이스, 엔진 전용 수학자료등 여러 라이브러리에서 사용되는 범용적인 자료들을 관리합니다.
* GOE_Engine: ECS 패턴을 구현하고, 씬(Scene)을 관리하며 각 시스템을 총괄하는 엔진의 핵심 로직을 포함합니다.
* GOE_Render: Directx관련 자료형/리소스, DirectXMath수학구조체등을 관리하고 D3D12 API를 직접 제어하여 실제 화면 출력을 담당합니다.
* GOE_AssetLoader: Assimp 라이브러리를 활용하여 FBX와 같은 3D 모델 파일을 로드하고, 엔진에서 사용할 수 있는 데이터로 변환합니다.
* GOE_Editor: ImGui를 통합하여 디버깅 및 에디터 기능을 제공합니다.

### 2.2 ECS (Entity-Component-System) 디자인 패턴 적용
현대 게임 엔진의 표준 아키텍처인 ECS 패턴을 도입하여 데이터와 로직을 효율적으로 분리했습니다.

Entity: 게임 세계에 존재하는 모든 오브젝트를 나타내는 고유 ID입니다.

Component: Transform, MeshRenderer, MovementUnit 등 오브젝트가 가질 수 있는 데이터 조각들입니다. 각 컴포넌트는 순수 데이터 컨테이너 역할을 합니다.

System: RenderSystem, MovementSystem, TransfromSystem 등 특정 컴포넌트들을 처리하여 실제 로직을 수행합니다. 이를 통해 코드의 재사용성과 유연성을 극대화했습니다.

Manager: EntityManager, ComponentManager<T>를 통해 엔티티와 컴포넌트를 효율적으로 생성, 조회 및 관리합니다.

C++

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
다. DirectX 12 기반의 렌더링 파이프라인
최신 그래픽스 API인 D3D12의 핵심 기능들을 직접 구현하여 로우레벨 렌더링 파이프라인에 대한 깊은 이해도를 보여줍니다.

명령 기반 렌더링: CommandQueue, CommandAllocator, CommandList를 사용하여 CPU와 GPU의 병렬 처리를 극대화하고 렌더링 명령을 효율적으로 관리합니다.

리소스 관리 및 동기화: Descriptor Heap, Resource Barrier를 통해 GPU 리소스의 상태를 명시적으로 관리하며, Fence를 이용한 CPU-GPU 동기화를 구현하여 안정적인 렌더링을 보장합니다.

셰이더 관리: 최신 셰이더 컴파일러인 DXC를 사용하여 HLSL 셰이더를 컴파일하며, Root Signature와 PSO(PipelineState Object)를 통해 셰이더 리소스 바인딩을 최적화합니다.

디버깅 및 프로파일링: PIX for Windows 와의 연동 코드를 포함하여 D3D12 렌더링 디버깅 및 성능 분석 능력을 갖추었습니다.

C++

// GOERenderer.cpp의 BeginRender 함수 예시: D3D12의 명령 기록 과정
void GOERenderer::BeginRender()
{
    WaitForFence(m_fenceValue);

    // 1. 커맨드 리스트 및 할당자를 리셋합니다.
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

    // 2. 렌더링 상태를 설정합니다 (루트 시그니처, 뷰포트 등).
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // 3. 리소스 배리어를 사용하여 렌더 타겟의 상태를 'PRESENT'에서 'RENDER_TARGET'으로 전환합니다.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrier);

    // 4. 렌더 타겟 및 깊이 버퍼를 설정하고 클리어합니다.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    const float clearColor[] = { .7f, .7f, .5f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
라. 개발 편의성을 고려한 디버깅 시스템
ImGui를 엔진에 통합하여 런타임 중에 엔진의 주요 상태(카메라 위치, 객체 정보 등)를 확인하고 수정할 수 있는 디버깅 UI를 구축했습니다. DebugManager를 싱글턴으로 구현하여 엔진의 어느 곳에서든 디버그 정보를 쉽게 출력할 수 있도록 설계했습니다.

3. 향후 발전 방향
애니메이션 시스템: 3D 모델의 스켈레탈 애니메이션 지원

물리 엔진: 충돌 감지 및 물리 반응 시뮬레이션

고급 렌더링 기술: PBR(Physically Based Rendering), 그림자 매핑, 후처리 효과 등

스크립팅 시스템: Lua 또는 다른 스크립트 언어를 통합하여 게임 로직의 빠른 프로토타이핑 지원

