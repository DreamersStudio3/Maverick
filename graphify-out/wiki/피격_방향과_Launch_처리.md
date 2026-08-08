# 피격 방향과 Launch 처리

> 15 nodes · cohesion 0.24

## Key Concepts

- **FMVResolvedHitData()** (29 connections) — `Source/Maverick/Public/Struct/MVHitTypes.h`
- **UMVHitReactionComponent::SnapOwnerYawToHitDirectionForLaunch()** (7 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionLogHitLaunchTrace()** (6 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionResolveHitSourceDirection()** (6 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionLogAirborneTrace()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionMakeYawSnapRotation()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionResolveHitDirection()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionShouldLogDirectionTrace()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::ResolveHitReactionDirection()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::ApplyHitReactionLaunch()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::HandleDamaged()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FVector** (3 connections)
- **UObject** (3 connections)
- **UMVHitReactionComponent::CanTriggerGroggy()** (2 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FRotator** (1 connections)

## Relationships

- [피격 반응 상태 관리](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%EC%83%81%ED%83%9C_%EA%B4%80%EB%A6%AC.md) (11 shared connections)
- [피격 회복 방향 추적](%ED%94%BC%EA%B2%A9_%ED%9A%8C%EB%B3%B5_%EB%B0%A9%ED%96%A5_%EC%B6%94%EC%A0%81.md) (7 shared connections)
- [적 회피 토큰 지급](%EC%A0%81_%ED%9A%8C%ED%94%BC_%ED%86%A0%ED%81%B0_%EC%A7%80%EA%B8%89.md) (4 shared connections)
- [캐릭터 스탯 관리](%EC%BA%90%EB%A6%AD%ED%84%B0_%EC%8A%A4%ED%83%AF_%EA%B4%80%EB%A6%AC.md) (3 shared connections)
- [피격 반응 Row 조회](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_Row_%EC%A1%B0%ED%9A%8C.md) (2 shared connections)
- [AI 피격 반응 태스크](AI_%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%ED%83%9C%EC%8A%A4%ED%81%AC.md) (2 shared connections)
- [캐릭터 공중 피격 추적](%EC%BA%90%EB%A6%AD%ED%84%B0_%EA%B3%B5%EC%A4%91_%ED%94%BC%EA%B2%A9_%EC%B6%94%EC%A0%81.md) (2 shared connections)
- [피격 판정 해석 파이프라인](%ED%94%BC%EA%B2%A9_%ED%8C%90%EC%A0%95_%ED%95%B4%EC%84%9D_%ED%8C%8C%EC%9D%B4%ED%94%84%EB%9D%BC%EC%9D%B8.md) (2 shared connections)
- [적 회피 토큰 부여](%EC%A0%81_%ED%9A%8C%ED%94%BC_%ED%86%A0%ED%81%B0_%EB%B6%80%EC%97%AC.md) (2 shared connections)
- [피격 리액션 입력·행 해석](%ED%94%BC%EA%B2%A9_%EB%A6%AC%EC%95%A1%EC%85%98_%EC%9E%85%EB%A0%A5%C2%B7%ED%96%89_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [AI 피격 진입 조건](AI_%ED%94%BC%EA%B2%A9_%EC%A7%84%EC%9E%85_%EC%A1%B0%EA%B1%B4.md) (1 shared connections)
- [적 캐릭터 전투 실행](%EC%A0%81_%EC%BA%90%EB%A6%AD%ED%84%B0_%EC%A0%84%ED%88%AC_%EC%8B%A4%ED%96%89.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVHitReactionComponent.cpp`
- `Source/Maverick/Public/Struct/MVHitTypes.h`

## Audit Trail

- EXTRACTED: 89 (99%)
- INFERRED: 1 (1%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*