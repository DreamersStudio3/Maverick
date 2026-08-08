# LockOnTarget 포커스 지점 해석

> 9 nodes

## Key Concepts

- **UTargetComponent::GetFocusPointLocation()** (5 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp`
- **ULockOnTargetComponent** (4 connections)
- **UTargetComponent::NotifyTargetReleased()** (3 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp`
- **UTargetComponent::GetSocketLocation()** (3 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp`
- **FVector** (3 connections)
- **UTargetComponent::GetCustomFocusPoint_Implementation()** (3 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp`
- **GetSocketLocation** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h`
- **GetCustomFocusPoint** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h`
- **K2_OnReleased** (2 connections) — `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h`

## Relationships

- [락온 대상 소켓 관리](%EB%9D%BD%EC%98%A8_%EB%8C%80%EC%83%81_%EC%86%8C%EC%BC%93_%EA%B4%80%EB%A6%AC.md) (5 shared connections)
- [락온 대상 포커스 설정](%EB%9D%BD%EC%98%A8_%EB%8C%80%EC%83%81_%ED%8F%AC%EC%BB%A4%EC%8A%A4_%EC%84%A4%EC%A0%95.md) (3 shared connections)
- [LockOnTarget 타깃 캡처 상태](LockOnTarget_%ED%83%80%EA%B9%83_%EC%BA%A1%EC%B2%98_%EC%83%81%ED%83%9C.md) (1 shared connections)

## Source Files

- `Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp`
- `Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h`

## Audit Trail

- EXTRACTED: 21 (78%)
- INFERRED: 6 (22%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*