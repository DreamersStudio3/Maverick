---
type: community
cohesion: 0.14
members: 15
---

# HUD 스탯 회복 연동

**Cohesion:** 0.14 - loosely connected
**Members:** 15 nodes

## Members
- [[FText_4]] - code
- [[MVBossHPBarWidget.cpp]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetBindToStatComponent()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleDamageAccumulated()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleDamageAccumulationReset()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleGroggyChanged()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleGroggyEnded()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleGroggyStarted()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetHandleHPChanged()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetInitializeBossBar()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetNativeDestruct()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetNativeOnInitialized()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetResetBossBar()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetUnbindStatComponent()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp
- [[UMVBossHPBarWidgetUpdateBossHP()]] - code - Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/HUD___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_스탯 컴포넌트 UI 연결]]
- 1 edge to [[_COMMUNITY_피격 방향과 Launch 처리]]

## Top bridge nodes
- [[UMVBossHPBarWidgetBindToStatComponent()]] - degree 3, connects to 1 community
- [[UMVBossHPBarWidgetHandleDamageAccumulated()]] - degree 2, connects to 1 community