# 피격 반응 Row 조회

> 21 nodes · cohesion 0.12

## Key Concepts

- **FMVHitReactionActionData** (10 connections) — `Source/Maverick/Components/MVHitReactionComponent.h`
- **MVHitReactionBuildAvailableRowNameLog()** (7 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FDataTableRowHandle** (5 connections)
- **UMVHitReactionComponent::GetActionData()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FMVHitReactionActionRow()** (5 connections) — `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`
- **MVHitReactionCopyBaseActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindBaseActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindHitReactionActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindRecoveryActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::ResolveRecoveryActionRowHandle()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FMVActionRow** (3 connections)
- **UMVHitReactionComponent::TryStartRecoveryAction()** (3 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MAVERICK_API** (2 connections)
- **UDataTable** (1 connections)
- **ActionRow** (1 connections) — `Source/Maverick/Components/MVHitReactionComponent.h`
- **ActionRowHandle** (1 connections) — `Source/Maverick/Components/MVHitReactionComponent.h`
- **Direction** (1 connections) — `Source/Maverick/Components/MVHitReactionComponent.h`
- **StartSection** (1 connections) — `Source/Maverick/Components/MVHitReactionComponent.h`
- **EMVHitReactionDirection** (1 connections)
- **FDataTableRowHandle** (1 connections)
- **FName** (1 connections)

## Relationships

- [피격 반응 상태 관리](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%EC%83%81%ED%83%9C_%EA%B4%80%EB%A6%AC.md) (8 shared connections)
- [피격 회복 방향 추적](%ED%94%BC%EA%B2%A9_%ED%9A%8C%EB%B3%B5_%EB%B0%A9%ED%96%A5_%EC%B6%94%EC%A0%81.md) (3 shared connections)
- [액션 행·몽타주 타입](%EC%95%A1%EC%85%98_%ED%96%89%C2%B7%EB%AA%BD%ED%83%80%EC%A3%BC_%ED%83%80%EC%9E%85.md) (2 shared connections)
- [피격 방향과 Launch 처리](%ED%94%BC%EA%B2%A9_%EB%B0%A9%ED%96%A5%EA%B3%BC_Launch_%EC%B2%98%EB%A6%AC.md) (2 shared connections)
- [피격 리액션 입력·행 해석](%ED%94%BC%EA%B2%A9_%EB%A6%AC%EC%95%A1%EC%85%98_%EC%9E%85%EB%A0%A5%C2%B7%ED%96%89_%ED%95%B4%EC%84%9D.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVHitReactionComponent.cpp`
- `Source/Maverick/Components/MVHitReactionComponent.h`
- `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`

## Audit Trail

- EXTRACTED: 68 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*