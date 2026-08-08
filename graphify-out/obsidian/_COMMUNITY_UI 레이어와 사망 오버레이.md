---
type: community
cohesion: 0.12
members: 22
---

# UI 레이어와 사망 오버레이

**Cohesion:** 0.12 - loosely connected
**Members:** 22 nodes

## Members
- [[APlayerController_5]] - code
- [[MVUILayerBase.cpp]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[MVUILayerBase.h]] - code - Source/Maverick/UI/System/MVUILayerBase.h
- [[TSubclassOf_4]] - code
- [[UCommonActivatableWidget]] - code
- [[UCommonActivatableWidget_1]] - code - Source/Maverick/UI/System/MVUILayerBase.h
- [[UCommonActivatableWidgetStack]] - code - Source/Maverick/UI/System/MVUILayerBase.h
- [[UMVDeathOverlayWindowBuildNativeWidgetTree()]] - code - Source/Maverick/UI/Window/MVDeathOverlayWindow.cpp
- [[UMVPopupBase_1]] - code
- [[UMVPopupBase_2]] - code - Source/Maverick/UI/System/MVUILayerBase.h
- [[UMVUILayerBaseAddFullScreenOverlayChild()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseAddWidgetLayerWidget()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseBuildNativeLayerTree()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseClearLayer()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseHideHUDWidgetWithFade()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseNativeOnInitialized()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBasePushPopup()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBasePushWindow()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseSetHUDByClass()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UMVUILayerBaseSetHUDWidget()]] - code - Source/Maverick/UI/System/MVUILayerBase.cpp
- [[UOverlay]] - code - Source/Maverick/UI/System/MVUILayerBase.h
- [[UUserWidget_4]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/UI___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_UI 기반 전환 유틸리티]]
- 1 edge to [[_COMMUNITY_스킬 슬롯 위젯 구성]]
- 1 edge to [[_COMMUNITY_상태 바 위젯]]
- 1 edge to [[_COMMUNITY_로딩 UI 테스트 명령]]
- 1 edge to [[_COMMUNITY_로딩 화면 입력 처리]]
- 1 edge to [[_COMMUNITY_사망 오버레이 페이드]]
- 1 edge to [[_COMMUNITY_PIE 액션 테스트 UI]]

## Top bridge nodes
- [[UOverlay]] - degree 7, connects to 3 communities
- [[UMVDeathOverlayWindowBuildNativeWidgetTree()]] - degree 3, connects to 2 communities
- [[MVUILayerBase.h]] - degree 5, connects to 1 community
- [[UMVUILayerBaseAddFullScreenOverlayChild()]] - degree 3, connects to 1 community