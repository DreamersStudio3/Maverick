# LockOnTarget 서브오브젝트 수명주기

> 10 nodes · cohesion 0.22

## Key Concepts

- **ULockOnTargetExtensionProxy()** (6 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h`
- **ULockOnTargetComponent::GetAllSubobjects()** (4 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp`
- **ULockOnTargetComponent::GetLifetimeReplicatedProps()** (3 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp`
- **TargetHandlerBase.h** (3 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h`
- **TArray** (2 connections)
- **ULockOnTargetComponent::DestroySubobject()** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp`
- **ULockOnTargetComponent::InitializeSubobject()** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp`
- **FFindTargetRequestParams()** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h`
- **FLifetimeProperty** (1 connections)
- **TInlineAllocator** (1 connections)

## Relationships

- [락온 대상 컴포넌트](%EB%9D%BD%EC%98%A8_%EB%8C%80%EC%83%81_%EC%BB%B4%ED%8F%AC%EB%84%8C%ED%8A%B8.md) (4 shared connections)
- [가중치 기반 타깃 선택](%EA%B0%80%EC%A4%91%EC%B9%98_%EA%B8%B0%EB%B0%98_%ED%83%80%EA%B9%83_%EC%84%A0%ED%83%9D.md) (1 shared connections)
- [락온 핵심 타입과 핸들러](%EB%9D%BD%EC%98%A8_%ED%95%B5%EC%8B%AC_%ED%83%80%EC%9E%85%EA%B3%BC_%ED%95%B8%EB%93%A4%EB%9F%AC.md) (1 shared connections)

## Source Files

- `Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp`
- `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h`

## Audit Trail

- EXTRACTED: 25 (96%)
- INFERRED: 1 (4%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*