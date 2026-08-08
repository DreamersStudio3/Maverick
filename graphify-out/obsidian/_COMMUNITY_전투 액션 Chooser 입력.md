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
- 4 edges to [[_COMMUNITY_FDataTableRowHandle]]
- 2 edges to [[_COMMUNITY_ExecuteAttackResolveActionCandidate]]
- 2 edges to [[_COMMUNITY_MVCombatComponent.cpp]]
- 1 edge to [[_COMMUNITY_MVEnemyDodgeActionTask.cpp]]
- 1 edge to [[_COMMUNITY_FMVDodgeInputContext]]
- 1 edge to [[_COMMUNITY_FMVSkillDataTableColumn]]
- 1 edge to [[_COMMUNITY_FMVCombatActionEvent]]
- 1 edge to [[_COMMUNITY_MVFinisherComponent.cpp]]
- 1 edge to [[_COMMUNITY_MVHitReactionComponent.cpp]]
- 1 edge to [[_COMMUNITY_MVHitReactionComponent.h]]
- 1 edge to [[_COMMUNITY_MVWeaponComponent.cpp]]

## Top bridge nodes
- [[UChooserTable]] - degree 7, connects to 5 communities
- [[FMVCombatActionTableInput]] - degree 15, connects to 3 communities
- [[UMVCombatComponentGetDataTableRowFromChooserTable_Implementation()]] - degree 4, connects to 3 communities
- [[UMVCombatComponentGetActionRowHandleFromChooserTable()]] - degree 5, connects to 2 communities
- [[UMVHitReactionComponentEvaluateHitReactionChooserActionRowHandle()]] - degree 3, connects to 2 communities