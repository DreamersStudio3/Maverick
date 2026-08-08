---
type: community
members: 10
---

# 회복 물약 런타임 상태

**Members:** 10 nodes

## Members
- [[AMVPlayerCharacter]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.h
- [[FMVHealingPotionRuntimeState()]] - code - Source/Maverick/Public/Struct/MVHealingPotionTypes.h
- [[MAVERICK_API_1]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.h
- [[MAVERICK_API_10]] - code
- [[MVHealingPotionTableTypes.h]] - code - Source/Maverick/Public/Tables/MVHealingPotionTableTypes.h
- [[MVHealingPotionTypes.h]] - code - Source/Maverick/Public/Struct/MVHealingPotionTypes.h
- [[MVPlayerConsumable.h]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.h
- [[UMVMainHUDWidgetHandleHealingPotionStateChanged()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVPlayerConsumableGetHealingPotionState()]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.cpp
- [[UWorld_5]] - code - Source/Maverick/Character/PC/Consumable/MVPlayerConsumable.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_플레이어 회복약 액션]]
- 1 edge to [[_COMMUNITY_액션 입력 처리 계약]]
- 1 edge to [[_COMMUNITY_메인 HUD 위젯 구성]]
- 1 edge to [[_COMMUNITY_회복 물약 기본값 해석]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]
- 1 edge to [[_COMMUNITY_메인 HUD 상태 바인딩]]

## Top bridge nodes
- [[MVHealingPotionTableTypes.h]] - degree 3, connects to 2 communities
- [[MVPlayerConsumable.h]] - degree 6, connects to 1 community
- [[MVHealingPotionTypes.h]] - degree 3, connects to 1 community
- [[UMVPlayerConsumableGetHealingPotionState()]] - degree 2, connects to 1 community
- [[UMVMainHUDWidgetHandleHealingPotionStateChanged()]] - degree 2, connects to 1 community