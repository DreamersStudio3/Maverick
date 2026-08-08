---
type: community
cohesion: 0.50
members: 4
---

# 필드 전환 사망 연동

**Cohesion:** 0.50 - moderately connected
**Members:** 4 nodes

## Members
- [[UMVDeathRespawnFlowInitialize()]] - code - Source/Maverick/System/MVDeathRespawnFlow.cpp
- [[UMVFieldTransitionSubsystem]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[UMVFieldTransitionSubsystemGet()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UObject_15]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_사망 부활 오케스트레이션]]
- 1 edge to [[_COMMUNITY_사망 부활 UI 흐름]]
- 1 edge to [[_COMMUNITY_필드 전환 관리]]

## Top bridge nodes
- [[UMVFieldTransitionSubsystem]] - degree 3, connects to 1 community
- [[UMVFieldTransitionSubsystemGet()]] - degree 3, connects to 1 community
- [[UMVDeathRespawnFlowInitialize()]] - degree 2, connects to 1 community