---
type: community
cohesion: 0.17
members: 13
---

# 전투 액션 행 해석

**Cohesion:** 0.17 - loosely connected
**Members:** 13 nodes

## Members
- [[FDataTableRowHandle_6]] - code
- [[MVCombatActionTableNameFromDataTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[TCHAR_6]] - code
- [[UDataTable_4]] - code
- [[UMVCombatComponentBuildSkillEntryFromRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsValidSkillActionRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentLoadFallbackAttackActionTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveActionRowHandleFromChooserTable()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseRow()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryBeginHeavyChargeAttack()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryMakeFallbackAttackActionRowHandle()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryStartActionWithAbility()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentUpdateLastBasicAttackSwingDirection()]] - code - Source/Maverick/Components/MVCombatComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 19 edges to [[_COMMUNITY_전투 액션 매핑]]
- 3 edges to [[_COMMUNITY_스킬 체인 런타임]]
- 2 edges to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 2 edges to [[_COMMUNITY_스킬 데이터와 비용]]
- 1 edge to [[_COMMUNITY_전투 액션 런타임 상태]]

## Top bridge nodes
- [[FDataTableRowHandle_6]] - degree 10, connects to 2 communities
- [[UMVCombatComponentTryStartActionWithAbility()]] - degree 5, connects to 2 communities
- [[UMVCombatComponentIsValidSkillActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVCombatComponentResolveActionRowHandleFromChooserTable()]] - degree 4, connects to 2 communities
- [[UMVCombatComponentResolveHeavyChargeEarlyReleaseRow()]] - degree 4, connects to 2 communities