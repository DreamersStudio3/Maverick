---
type: community
cohesion: 0.18
members: 11
---

# 플레이어 상태 UI 생명주기

**Cohesion:** 0.18 - loosely connected
**Members:** 11 nodes

## Members
- [[MVPlayerStatusWidget.cpp]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetBuildNativeWidgetTree()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetConfigureStatusBars()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetHandleHPChanged()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetHandleMPChanged()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetHandleStaminaChanged()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetHandleStatRecentLossHoldChanged()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetNativeConstruct()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetNativeDestruct()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetNativeOnInitialized()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp
- [[UMVPlayerStatusWidgetUnbindStatComponent()]] - code - Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__UI_
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_상태 바 위젯 선언]]
- 1 edge to [[_COMMUNITY_스탯 컴포넌트 UI 연결]]

## Top bridge nodes
- [[MVPlayerStatusWidget.cpp]] - degree 12, connects to 2 communities