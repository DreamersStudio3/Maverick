---
type: community
cohesion: 0.12
members: 22
---

# FDataTableRowHandle

**Cohesion:** 0.12 - loosely connected
**Members:** 22 nodes

## Members
- [[FDataTableRowHandle_7]] - code
- [[FName_35]] - code
- [[MVCombatActionTableNameFromDataTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatAppendRowCandidate()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[MVCombatMakeFallbackAttackRowCandidates()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[TArray_13]] - code
- [[TCHAR_6]] - code
- [[UDataTable_4]] - code
- [[UMVCombatComponentBuildSkillEntryFromRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHandleActionEnded()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsCurrentAbilityAction()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsHeavyChargeBasicAttackMapKey()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsValidSkillActionRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentLoadFallbackAttackActionTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResetOtherBasicAttackChains()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveActionRowHandleFromChooserTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseRow()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryBeginHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryEarlyReleaseHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryMakeFallbackAttackActionRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryStartActionWithAbility()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUpdateLastBasicAttackSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/FDataTableRowHandle
SORT file.name ASC
```

## Connections to other communities
- 35 edges to [[_COMMUNITY_MVCombatComponent.cpp]]
- 4 edges to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 3 edges to [[_COMMUNITY_FMVSkillEntry]]
- 2 edges to [[_COMMUNITY_FMVSkillDataTableColumn]]
- 1 edge to [[_COMMUNITY_FMVCombatActionEvent_1]]

## Top bridge nodes
- [[FName_35]] - degree 23, connects to 2 communities
- [[FDataTableRowHandle_7]] - degree 10, connects to 2 communities
- [[UMVCombatComponentTryStartActionWithAbility()]] - degree 5, connects to 2 communities
- [[UMVCombatComponentIsValidSkillActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVCombatComponentResolveActionRowHandleFromChooserTable()]] - degree 4, connects to 2 communities