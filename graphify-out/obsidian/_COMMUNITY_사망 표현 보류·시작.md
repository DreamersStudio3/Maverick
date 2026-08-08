---
type: community
cohesion: 0.33
members: 6
---

# 사망 표현 보류·시작

**Cohesion:** 0.33 - loosely connected
**Members:** 6 nodes

## Members
- [[FMVDeathContext()]] - code - Source/Maverick/Components/MVStatComponent.h
- [[UMVDeathComponentBeginDeathPresentation()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentClearDeferredDeathPresentation()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentHandleDeathStarted()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentTryDeferDeathPresentationUntilHitReactionEnds()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentTryStartDeathAction()]] - code - Source/Maverick/Components/MVDeathComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_캐릭터 사망 표현]]
- 2 edges to [[_COMMUNITY_사망 액션 행 이름 해석]]
- 1 edge to [[_COMMUNITY_사망 액션 포즈 해석]]
- 1 edge to [[_COMMUNITY_HUD 스탯 회복 연동]]

## Top bridge nodes
- [[FMVDeathContext()]] - degree 9, connects to 3 communities
- [[UMVDeathComponentBeginDeathPresentation()]] - degree 2, connects to 1 community
- [[UMVDeathComponentClearDeferredDeathPresentation()]] - degree 2, connects to 1 community
- [[UMVDeathComponentHandleDeathStarted()]] - degree 2, connects to 1 community
- [[UMVDeathComponentTryDeferDeathPresentationUntilHitReactionEnds()]] - degree 2, connects to 1 community