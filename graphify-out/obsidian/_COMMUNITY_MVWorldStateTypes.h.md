---
type: community
cohesion: 0.18
members: 11
---

# MVWorldStateTypes.h

**Cohesion:** 0.18 - loosely connected
**Members:** 11 nodes

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
- [[UMVWorldStateSubsystemResetSaveData()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp
- [[UMVWorldStateSubsystemTryGetLastCheckpoint()]] - code - Source/Maverick/System/MVWorldStateSubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/MVWorldStateTypesh
SORT file.name ASC
```

## Connections to other communities
- 6 edges to [[_COMMUNITY_MVWorldStateSubsystem.cpp]]
- 1 edge to [[_COMMUNITY_MVFieldTransitionSubsystem.cpp]]
- 1 edge to [[_COMMUNITY_UMVWorldStateSubsystem]]

## Top bridge nodes
- [[MVWorldStateTypes.h]] - degree 8, connects to 2 communities
- [[UMVWorldStateSubsystemApplySaveData()]] - degree 2, connects to 1 community
- [[UMVWorldStateSubsystemResetSaveData()]] - degree 2, connects to 1 community
- [[UMVWorldStateSubsystemTryGetLastCheckpoint()]] - degree 2, connects to 1 community
- [[FMVFieldObjectSaveData()]] - degree 2, connects to 1 community