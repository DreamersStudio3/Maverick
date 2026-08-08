---
type: community
cohesion: 0.19
members: 15
---

# 액션 입력 버퍼링

**Cohesion:** 0.19 - loosely connected
**Members:** 15 nodes

## Members
- [[EMVActionInputPhase]] - code - Source/Maverick/Public/Enum/MVActionInputPhase.h
- [[FGameplayTag_10]] - code
- [[FVector2D_9]] - code
- [[UMETA_5]] - code - Source/Maverick/Public/Enum/MVActionInputPhase.h
- [[UMVInputManagerComponentBufferHoldActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentBufferInstantActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentBuildActionInputSnapshot()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentClampActionControllerSpaceInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentSubmitActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentSubmitHoldActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentTryGetBufferedActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentTryGetRecentActionMovementInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentTryRouteActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[UMVInputManagerComponentTryRouteHoldActionInput()]] - code - Source/Maverick/Components/MVInputManagerComponent.cpp
- [[uint8_9]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__
SORT file.name ASC
```

## Connections to other communities
- 10 edges to [[_COMMUNITY_입력 차단 회복 취소]]
- 1 edge to [[_COMMUNITY_전투 액션 입력 처리]]
- 1 edge to [[_COMMUNITY_피격 리액션 입력·행 해석]]
- 1 edge to [[_COMMUNITY_액션 입력 방향 해석]]
- 1 edge to [[_COMMUNITY_컨트롤러 기준 이동 입력]]
- 1 edge to [[_COMMUNITY_액션 전환 입력 관리]]

## Top bridge nodes
- [[EMVActionInputPhase]] - degree 8, connects to 3 communities
- [[FVector2D_9]] - degree 10, connects to 2 communities
- [[UMVInputManagerComponentBufferHoldActionInput()]] - degree 4, connects to 1 community
- [[UMVInputManagerComponentTryRouteHoldActionInput()]] - degree 4, connects to 1 community
- [[UMVInputManagerComponentBufferInstantActionInput()]] - degree 3, connects to 1 community