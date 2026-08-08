---
type: community
cohesion: 0.33
members: 11
---

# 회피 입력 방향 구성

**Cohesion:** 0.33 - loosely connected
**Members:** 11 nodes

## Members
- [[DodgeClampControllerSpaceInput()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[DodgeResolveDirectionFromControllerSpaceInput()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[ELocomotionDirection_1]] - code
- [[FVector2D_4]] - code
- [[IsBackwardDiagonalDodgeDirection()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[IsDiagonalDodgeDirection()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[ResolveDodgeChooserDirection()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[ResolveDodgeEightWayDirection()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[ResolveDodgeFacingDirection()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeCacheControllerSpaceMovementInput()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVPlayerDodgeMakeDodgeInputContext()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 11 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 9 edges to [[_COMMUNITY_회피 방향 벡터 변환]]
- 1 edge to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_회피 입력 컨텍스트]]

## Top bridge nodes
- [[UMVPlayerDodgeMakeDodgeInputContext()]] - degree 11, connects to 3 communities
- [[ELocomotionDirection_1]] - degree 9, connects to 3 communities
- [[FVector2D_4]] - degree 7, connects to 2 communities
- [[ResolveDodgeFacingDirection()]] - degree 6, connects to 2 communities
- [[DodgeClampControllerSpaceInput()]] - degree 5, connects to 2 communities