---
type: community
cohesion: 0.50
members: 4
---

# 컨트롤러 기준 이동 입력

**Cohesion:** 0.50 - moderately connected
**Members:** 4 nodes

## Members
- [[FVector_15]] - code
- [[UMVInputManagerComponentHandleOwnerMovementInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentResolveControllerSpaceInputFromWorldDirection()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentUpdateActionMovementInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_입력 차단 회복 취소]]
- 1 edge to [[_COMMUNITY_액션 입력 버퍼링]]

## Top bridge nodes
- [[UMVInputManagerComponentResolveControllerSpaceInputFromWorldDirection()]] - degree 3, connects to 2 communities
- [[UMVInputManagerComponentHandleOwnerMovementInput()]] - degree 2, connects to 1 community
- [[UMVInputManagerComponentUpdateActionMovementInput()]] - degree 2, connects to 1 community