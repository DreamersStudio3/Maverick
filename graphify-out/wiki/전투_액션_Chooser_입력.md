# 전투 액션 Chooser 입력

> 15 nodes · cohesion 0.14

## Key Concepts

- **FMVCombatActionTableInput** (15 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **UChooserTable** (7 connections) — `Source/Maverick/Components/MVCombatComponent.h`
- **UMVCombatComponent::GetActionRowHandleFromChooserTable()** (5 connections) — `Source/Maverick/Components/MVCombatComponent.cpp`
- **UMVCombatComponent::GetDataTableRowFromChooserTable_Implementation()** (4 connections) — `Source/Maverick/Components/MVCombatComponent.cpp`
- **UMVHitReactionComponent::EvaluateHitReactionChooserActionRowHandle()** (3 connections) — `Source/Maverick/Components/MVHitReactionComponent.cpp`
- **.SetActionType()** (2 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **FGameplayTag** (2 connections)
- **ActionType** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **ActionTypeTags** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **CurrentWeaponStyle** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **.FMVCombatActionTableInput()** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **.RefreshActionTypeTags()** (1 connections) — `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`
- **EMVEquippedStyle** (1 connections)
- **FGameplayTagContainer** (1 connections)
- **UPROPERTY** (1 connections)

## Relationships

- [전투 액션 매핑](%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%EB%A7%A4%ED%95%91.md) (4 shared connections)
- [AI 공격 액션 후보 해석](AI_%EA%B3%B5%EA%B2%A9_%EC%95%A1%EC%85%98_%ED%9B%84%EB%B3%B4_%ED%95%B4%EC%84%9D.md) (2 shared connections)
- [전투 액션 행 해석](%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%ED%96%89_%ED%95%B4%EC%84%9D.md) (2 shared connections)
- [AI 적 회피 실행](AI_%EC%A0%81_%ED%9A%8C%ED%94%BC_%EC%8B%A4%ED%96%89.md) (1 shared connections)
- [플레이어 회피 방향 해석](%ED%94%8C%EB%A0%88%EC%9D%B4%EC%96%B4_%ED%9A%8C%ED%94%BC_%EB%B0%A9%ED%96%A5_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [전투 액션 런타임 상태](%EC%A0%84%ED%88%AC_%EC%95%A1%EC%85%98_%EB%9F%B0%ED%83%80%EC%9E%84_%EC%83%81%ED%83%9C.md) (1 shared connections)
- [피니셔 실행과 워핑](%ED%94%BC%EB%8B%88%EC%85%94_%EC%8B%A4%ED%96%89%EA%B3%BC_%EC%9B%8C%ED%95%91.md) (1 shared connections)
- [스킬 데이터와 비용](%EC%8A%A4%ED%82%AC_%EB%8D%B0%EC%9D%B4%ED%84%B0%EC%99%80_%EB%B9%84%EC%9A%A9.md) (1 shared connections)
- [피격 반응 상태 관리](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_%EC%83%81%ED%83%9C_%EA%B4%80%EB%A6%AC.md) (1 shared connections)
- [피격 반응 Row 조회](%ED%94%BC%EA%B2%A9_%EB%B0%98%EC%9D%91_Row_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [무기 장착 시각화](%EB%AC%B4%EA%B8%B0_%EC%9E%A5%EC%B0%A9_%EC%8B%9C%EA%B0%81%ED%99%94.md) (1 shared connections)

## Source Files

- `Source/Maverick/Components/MVCombatComponent.cpp`
- `Source/Maverick/Components/MVCombatComponent.h`
- `Source/Maverick/Components/MVHitReactionComponent.cpp`
- `Source/Maverick/Public/Struct/MVCombatActionTableInput.h`

## Audit Trail

- EXTRACTED: 46 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*