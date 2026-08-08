---
type: community
cohesion: 0.29
members: 8
---

# 회복 물약 액션 식별

**Cohesion:** 0.29 - loosely connected
**Members:** 8 nodes

## Members
- [[FDataTableRowHandle_3]] - code
- [[FName_31]] - code
- [[UMVPlayerConsumableHandleActionEnded()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableHandleActionStarted()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableIsHealingPotionFailedAction()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableIsHealingPotionUseAction()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableResolveHealingPotionActionTableName()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableTryStartHealingPotionAction()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 6 edges to [[_COMMUNITY_플레이어 회복약 액션]]
- 1 edge to [[_COMMUNITY_회복 물약 기본값 해석]]

## Top bridge nodes
- [[FDataTableRowHandle_3]] - degree 3, connects to 1 community
- [[UMVPlayerConsumableResolveHealingPotionActionTableName()]] - degree 3, connects to 1 community
- [[UMVPlayerConsumableTryStartHealingPotionAction()]] - degree 3, connects to 1 community
- [[UMVPlayerConsumableHandleActionEnded()]] - degree 2, connects to 1 community
- [[UMVPlayerConsumableHandleActionStarted()]] - degree 2, connects to 1 community