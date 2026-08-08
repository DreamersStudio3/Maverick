---
type: community
cohesion: 0.33
members: 6
---

# 월드 상태 서브시스템 접근

**Cohesion:** 0.33 - loosely connected
**Members:** 6 nodes

## Members
- [[MVWorldStateSubsystem.h]] - code - Source/Maverick/System/MVWorldStateSubsystem.h
- [[UMVFieldTransitionSubsystemGetWorldState()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVQuestSubsystemGetWorldState()]] - code - Source/Maverick/System/MVQuestSubsystem.cpp
- [[UMVWorldStateSubsystem()]] - code - Source/Maverick/System/MVWorldStateSubsystem.h
- [[UMVWorldStateSubsystemGet()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UObject_17]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_필드 전환 관리]]
- 1 edge to [[_COMMUNITY_퀘스트 상태 API]]
- 1 edge to [[_COMMUNITY_월드 상태 저장]]
- 1 edge to [[_COMMUNITY_월드 상태 저장 레코드]]

## Top bridge nodes
- [[UMVWorldStateSubsystemGet()]] - degree 3, connects to 1 community
- [[UMVFieldTransitionSubsystemGetWorldState()]] - degree 2, connects to 1 community
- [[UMVQuestSubsystemGetWorldState()]] - degree 2, connects to 1 community
- [[MVWorldStateSubsystem.h]] - degree 2, connects to 1 community