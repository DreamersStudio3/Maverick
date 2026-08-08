---
type: community
cohesion: 0.14
members: 15
---

# 전투 액션 Chooser 입력

**Cohesion:** 0.14 - loosely connected
**Members:** 15 nodes

## Members
- [[dot-FMVCombatActionTableInput()]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[dot-RefreshActionTypeTags()]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[dot-SetActionType()]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[ActionType_2]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[ActionTypeTags]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[CurrentWeaponStyle]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[EMVEquippedStyle_4]] - code
- [[FGameplayTag_14]] - code
- [[FGameplayTagContainer_1]] - code
- [[FMVCombatActionTableInput]] - code - Source/Maverick/Public/Struct/MVCombatActionTableInput.h
- [[UChooserTable]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[UMVCombatComponentGetActionRowHandleFromChooserTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentGetDataTableRowFromChooserTable_Implementation()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVHitReactionComponentEvaluateHitReactionChooserActionRowHandle()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UPROPERTY_11]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__Chooser_
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_전투 액션 매핑]]
- 2 edges to [[_COMMUNITY_AI 공격 액션 후보 해석]]
- 2 edges to [[_COMMUNITY_전투 액션 행 해석]]
- 1 edge to [[_COMMUNITY_AI 적 회피 실행]]
- 1 edge to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 1 edge to [[_COMMUNITY_스킬 데이터와 비용]]
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]
- 1 edge to [[_COMMUNITY_피니셔 실행과 워핑]]
- 1 edge to [[_COMMUNITY_피격 반응 상태 관리]]
- 1 edge to [[_COMMUNITY_피격 반응 Row 조회]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]

## Top bridge nodes
- [[UChooserTable]] - degree 7, connects to 5 communities
- [[FMVCombatActionTableInput]] - degree 15, connects to 3 communities
- [[UMVCombatComponentGetActionRowHandleFromChooserTable()]] - degree 5, connects to 2 communities
- [[UMVCombatComponentGetDataTableRowFromChooserTable_Implementation()]] - degree 4, connects to 2 communities
- [[UMVHitReactionComponentEvaluateHitReactionChooserActionRowHandle()]] - degree 3, connects to 2 communities