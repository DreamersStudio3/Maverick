---
type: community
members: 4
---

# 상호작용 후보 선택

**Members:** 4 nodes

## Members
- [[UMVPlayerInteractionDetectorFindCandidateIndex()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[UMVPlayerInteractionDetectorSelectInteractableByOffset()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[UMVPlayerInteractionDetectorSetSelectedCandidateIndex()]] - code - Source/Maverick/Character/PC/InteractionDetector/MVPlayerInteractionDetector.cpp
- [[int32_12]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_플레이어 상호작용 감지]]
- 1 edge to [[_COMMUNITY_상호작용 가능성·포커스 판정]]

## Top bridge nodes
- [[UMVPlayerInteractionDetectorFindCandidateIndex()]] - degree 3, connects to 2 communities
- [[UMVPlayerInteractionDetectorSelectInteractableByOffset()]] - degree 2, connects to 1 community
- [[UMVPlayerInteractionDetectorSetSelectedCandidateIndex()]] - degree 2, connects to 1 community