---
type: community
cohesion: 0.15
members: 12
---

# 전투 액션 입력 처리

**Cohesion:** 0.15 - loosely connected
**Members:** 12 nodes

## Members
- [[EMVEquippedStyle_2]] - code
- [[FGameplayTag_6]] - code
- [[FVector2D_6]] - code
- [[UMVAbilityBase_2]] - code
- [[UMVCombatComponentChangeWeapon()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentChooseTryCombatAction()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentHandleAbilityEnded()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsCombatActionInputTag()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentIsHeavyChargeActionInputTag()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentMakeActionTypeGameplayTag()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryHandleActionInput()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[UMVCombatComponentTryHandleHoldActionInput()]] - code - Source/Maverick/Components/MVCombatComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 10 edges to [[_COMMUNITY_전투 액션 매핑]]
- 1 edge to [[_COMMUNITY_액션 입력 버퍼링]]

## Top bridge nodes
- [[UMVCombatComponentTryHandleHoldActionInput()]] - degree 4, connects to 2 communities
- [[FGameplayTag_6]] - degree 7, connects to 1 community
- [[UMVCombatComponentMakeActionTypeGameplayTag()]] - degree 3, connects to 1 community
- [[UMVCombatComponentTryHandleActionInput()]] - degree 3, connects to 1 community
- [[UMVCombatComponentChangeWeapon()]] - degree 2, connects to 1 community