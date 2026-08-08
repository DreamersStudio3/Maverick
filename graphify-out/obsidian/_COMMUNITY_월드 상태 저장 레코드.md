---
type: community
cohesion: 0.14
members: 14
---

# 월드 상태 저장 레코드

**Cohesion:** 0.14 - loosely connected
**Members:** 14 nodes

## Members
- [[FMVCheckpointSaveData()]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[FMVFieldObjectSaveData()]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[FMVQuestSaveData()]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[FMVWorldFlagSaveData()]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[FMVWorldSaveData()]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[MVWorldSaveGame.h]] - code - Source/Maverick/System/MVWorldSaveGame.h
- [[MVWorldStateTypes.h]] - code - Source/Maverick/System/MVWorldStateTypes.h
- [[UMVWorldSaveGame()]] - code - Source/Maverick/System/MVWorldSaveGame.h
- [[UMVWorldStateSubsystemApplySaveData()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemFindFieldObjectRecord()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemFindQuestRecord()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemFindWorldFlagRecord()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemResetSaveData()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemTryGetLastCheckpoint()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 9 edges to [[_COMMUNITY_월드 상태 저장]]
- 1 edge to [[_COMMUNITY_필드 전환 관리]]
- 1 edge to [[_COMMUNITY_월드 상태 서브시스템 접근]]

## Top bridge nodes
- [[MVWorldStateTypes.h]] - degree 8, connects to 2 communities
- [[UMVWorldStateSubsystemFindFieldObjectRecord()]] - degree 3, connects to 1 community
- [[UMVWorldStateSubsystemFindQuestRecord()]] - degree 3, connects to 1 community
- [[UMVWorldStateSubsystemFindWorldFlagRecord()]] - degree 3, connects to 1 community
- [[UMVWorldStateSubsystemApplySaveData()]] - degree 2, connects to 1 community