---
type: community
members: 6
---

# 회복 물약 기본값 해석

**Members:** 6 nodes

## Members
- [[ApplyMVPlayerConsumableHealingPotionDataRowDefaults()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[ApplyMVPlayerConsumableHealingPotionDefaults()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[FMVHealingPotionTableRow()]] - code - Source/Maverick/Public/Tables/MVHealingPotionTableTypes.h
- [[MakeMVPlayerConsumableDefaultHealingPotionData()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableResolveHealingPotionData()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UMVPlayerConsumableUMVPlayerConsumable()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 6 edges to [[_COMMUNITY_플레이어 회복약 액션]]
- 1 edge to [[_COMMUNITY_회복 물약 런타임 상태]]

## Top bridge nodes
- [[MakeMVPlayerConsumableDefaultHealingPotionData()]] - degree 4, connects to 1 community
- [[ApplyMVPlayerConsumableHealingPotionDataRowDefaults()]] - degree 4, connects to 1 community
- [[ApplyMVPlayerConsumableHealingPotionDefaults()]] - degree 4, connects to 1 community
- [[UMVPlayerConsumableUMVPlayerConsumable()]] - degree 4, connects to 1 community
- [[UMVPlayerConsumableResolveHealingPotionData()]] - degree 4, connects to 1 community