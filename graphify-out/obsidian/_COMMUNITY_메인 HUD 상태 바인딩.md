---
type: community
cohesion: 0.18
members: 12
---

# 메인 HUD 상태 바인딩

**Cohesion:** 0.18 - loosely connected
**Members:** 12 nodes

## Members
- [[FText_5]] - code
- [[MVMainHUDWidget.cpp]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetApplyHealingPotionQuickSlotView()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetBindBossStatus()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetBindPlayerConsumable()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetEnsurePlayerSkillHUD()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetHideBossHPBar()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetInitBossStatus()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetNativeOnInitialized()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetRefreshHUD()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVMainHUDWidgetUpdateBossStatus()]] - code - Source/Maverick/UI/HUD/MVMainHUDWidget.cpp
- [[UMVPlayerConsumable_2]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/_HUD__
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_메인 HUD 위젯 구성]]
- 1 edge to [[_COMMUNITY_HUD 스탯 회복 연동]]
- 1 edge to [[_COMMUNITY_회복 물약 런타임 상태]]

## Top bridge nodes
- [[MVMainHUDWidget.cpp]] - degree 11, connects to 2 communities
- [[UMVMainHUDWidgetBindBossStatus()]] - degree 3, connects to 1 community
- [[UMVMainHUDWidgetEnsurePlayerSkillHUD()]] - degree 2, connects to 1 community