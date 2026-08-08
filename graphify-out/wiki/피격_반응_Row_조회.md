# 피격 반응 Row 조회

> 17 nodes · cohesion 0.16

## Key Concepts

- **MVHitReactionBuildAvailableRowNameLog()** (7 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FDataTableRowHandle** (5 connections)
- **UMVHitReactionComponent::GetActionData()** (5 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MVHitReactionActionTableTypes.h** (5 connections) — `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`
- **FMVHitReactionActionRow()** (5 connections) — `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`
- **FMVHitReactionActionRowHandle()** (5 connections) — `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`
- **MVHitReactionCopyBaseActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindBaseActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindHitReactionActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::FindRecoveryActionRow()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::MakeHitReactionActionRowHandleFromNames()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::ResolveHitReactionActionRowHandle()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **UMVHitReactionComponent::ResolveRecoveryActionRowHandle()** (4 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **FMVActionRow** (3 connections)
- **UMVHitReactionComponent::TryStartRecoveryAction()** (3 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **MAVERICK_API** (2 connections)
- **UDataTable** (1 connections)

## Relationships

- [피격 반응 상태 관리](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%EC%83%81%ED%83%9C_%EA%B4%80%EB%A6%AC.md) (10 shared connections)
- [피격 회복 방향 추적](%ED%94%BC%EA%B2%A9_%ED%9A%8C%EB%B3%B5_%EB%B0%A9%ED%96%A5_%EC%B6%94%EC%A0%81.md) (4 shared connections)
- [피격 방향과 Launch 처리](%ED%94%BC%EA%B2%A9_%EB%B0%A9%ED%96%A5%EA%B3%BC_Launch_%EC%B2%98%EB%A6%AC.md) (2 shared connections)
- [피격 리액션 액션 데이터](%ED%94%BC%EA%B2%A9_%EB%A6%AC%EC%95%A1%EC%85%98_%EC%95%A1%EC%85%98_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (2 shared connections)
- [피격 리액션 입력·행 해석](%ED%94%BC%EA%B2%A9_%EB%A6%AC%EC%95%A1%EC%85%98_%EC%9E%85%EB%A0%A5%C2%B7%ED%96%89_%ED%95%B4%EC%84%9D.md) (2 shared connections)
- [테이블 기반 UI·스탯 조회](%ED%85%8C%EC%9D%B4%EB%B8%94_%EA%B8%B0%EB%B0%98_UI%C2%B7%EC%8A%A4%ED%83%AF_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [피격 시스템 공용 타입](%ED%94%BC%EA%B2%A9_%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [시트 레시피 사양 로더](%EC%8B%9C%ED%8A%B8_%EB%A0%88%EC%8B%9C%ED%94%BC_%EC%82%AC%EC%96%91_%EB%A1%9C%EB%8D%94.md) (1 shared connections)
- [액션 행·몽타주 타입](%EC%95%A1%EC%85%98_%ED%96%89%C2%B7%EB%AA%BD%ED%83%80%EC%A3%BC_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [전투 액션 Chooser 입력](%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_Chooser_%EC%9E%85%EB%A0%A5.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVHitReactionComponent.cpp`
- `Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h`

## Audit Trail

- EXTRACTED: 69 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*