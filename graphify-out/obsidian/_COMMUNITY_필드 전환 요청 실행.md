---
type: community
cohesion: 0.29
members: 7
---

# 필드 전환 요청 실행

**Cohesion:** 0.29 - loosely connected
**Members:** 7 nodes

## Members
- [[FMVFieldTransitionRequest()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.h
- [[UMVFieldTransitionSubsystemApplyTransitionDestination()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResetTransitionState()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResetWorldActorsForTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemStartTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemTryPreparePostTransitionAction()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[int32_26]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_필드 전환 관리]]
- 1 edge to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_필드 전환 요청 식별]]
- 1 edge to [[_COMMUNITY_필드 전환 서브시스템 계약]]

## Top bridge nodes
- [[FMVFieldTransitionRequest()]] - degree 7, connects to 2 communities
- [[UMVFieldTransitionSubsystemTryPreparePostTransitionAction()]] - degree 3, connects to 2 communities
- [[UMVFieldTransitionSubsystemResetWorldActorsForTransition()]] - degree 3, connects to 1 community
- [[UMVFieldTransitionSubsystemApplyTransitionDestination()]] - degree 2, connects to 1 community
- [[UMVFieldTransitionSubsystemResetTransitionState()]] - degree 2, connects to 1 community