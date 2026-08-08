---
type: community
members: 4
---

# 상호작용 후보 판정

**Members:** 4 nodes

## Members
- [[FMVInteractionCandidate]] - code
- [[FVector_12]] - code
- [[UMVPlayerInteractionDetectorHasLineOfSight()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[UMVPlayerInteractionDetectorTryBuildCandidate()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_플레이어 상호작용 감지]]
- 1 edge to [[_COMMUNITY_플레이어 상호작용 탐지]]

## Top bridge nodes
- [[UMVPlayerInteractionDetectorTryBuildCandidate()]] - degree 4, connects to 2 communities
- [[UMVPlayerInteractionDetectorHasLineOfSight()]] - degree 2, connects to 1 community