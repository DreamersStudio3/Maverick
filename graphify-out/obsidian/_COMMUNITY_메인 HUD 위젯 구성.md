---
type: community
cohesion: 0.22
members: 11
---

# 메인 HUD 위젯 구성

**Cohesion:** 0.22 - loosely connected
**Members:** 11 nodes

## Members
- [[MVMainHUDWidget.h]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVBossHPBarWidget]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVCombatComponent_2]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVCurrencyStatusWidget]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVMainHUDWidget()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVMainHUDWidgetBuildNativeWidgetTree()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVPlayerConsumable_3]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVPlayerSkillHUDWidget]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVPlayerStatusWidget]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVQuickSlotWidget]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h
- [[UMVStatComponent_5]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/_HUD__
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_메인 HUD 상태 바인딩]]
- 1 edge to [[_COMMUNITY_회복 물약 런타임 상태]]

## Top bridge nodes
- [[MVMainHUDWidget.h]] - degree 10, connects to 1 community
- [[UMVMainHUDWidgetBuildNativeWidgetTree()]] - degree 4, connects to 1 community
- [[UMVPlayerSkillHUDWidget]] - degree 3, connects to 1 community