---
type: community
cohesion: 0.40
members: 5
---

# 회복 물약 런타임 상태

**Cohesion:** 0.40 - moderately connected
**Members:** 5 nodes

## Members
- [[FMVHealingPotionRuntimeState()]] - code - Source/Maverick/Public/Struct/MVHealingPotionTypes.h
- [[MAVERICK_API_10]] - code
- [[MVHealingPotionTypes.h]] - code - Source/Maverick/Public/Struct/MVHealingPotionTypes.h
- [[UMVMainHUDWidgetHandleHealingPotionStateChanged()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVPlayerConsumableGetHealingPotionState()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_플레이어 회복약 액션]]
- 1 edge to [[_COMMUNITY_플레이어 회피 입력 계약]]
- 1 edge to [[_COMMUNITY_메인 HUD 위젯 구성]]
- 1 edge to [[_COMMUNITY_메인 HUD 상태 바인딩]]

## Top bridge nodes
- [[MVHealingPotionTypes.h]] - degree 3, connects to 2 communities
- [[UMVPlayerConsumableGetHealingPotionState()]] - degree 2, connects to 1 community
- [[UMVMainHUDWidgetHandleHealingPotionStateChanged()]] - degree 2, connects to 1 community