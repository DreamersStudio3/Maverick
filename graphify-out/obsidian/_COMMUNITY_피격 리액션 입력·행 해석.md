---
type: community
cohesion: 0.21
members: 13
---

# 피격 리액션 입력·행 해석

**Cohesion:** 0.21 - loosely connected
**Members:** 13 nodes

## Members
- [[FGameplayTag_9]] - code
- [[FName_39]] - code
- [[FVector2D_8]] - code
- [[UMVHitReactionComponentHandleActionEnded()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeGroggyActionRowName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeGroggyActionTableName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionTableName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveCharacterIndexCode()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveHitReactionActionTableName()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryConsumeRecoveryInput()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryConsumeRecoveryMovementInput()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryHandleActionInput()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryHandleHoldActionInput()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 10 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 7 edges to [[_COMMUNITY_피격 회복 방향 추적]]
- 2 edges to [[_COMMUNITY_피격 반응 Row 조회]]
- 1 edge to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 1 edge to [[_COMMUNITY_액션 입력 버퍼링]]

## Top bridge nodes
- [[FName_39]] - degree 12, connects to 3 communities
- [[UMVHitReactionComponentTryHandleHoldActionInput()]] - degree 5, connects to 3 communities
- [[UMVHitReactionComponentTryHandleActionInput()]] - degree 4, connects to 2 communities
- [[FGameplayTag_9]] - degree 8, connects to 1 community
- [[UMVHitReactionComponentMakeGroggyActionRowName()]] - degree 3, connects to 1 community