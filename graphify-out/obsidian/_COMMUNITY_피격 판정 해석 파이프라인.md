---
type: community
cohesion: 0.33
members: 13
---

# 피격 판정 해석 파이프라인

**Cohesion:** 0.33 - loosely connected
**Members:** 13 nodes

## Members
- [[FMVHitResolveRequest()]] - code - Source/Maverick/Public/Struct/MVHitTypes.h
- [[FVector_13]] - code
- [[MVHitResolverLogAirborneTrace()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[MVHitResolverLogHitLaunchTrace()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[MVHitResolverShouldLogAirborneTrace()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[MVHitResolverSubsystem.cpp]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[MVHitResolverTryNormalize2D()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[MVHitResolverTryResolveAttackerToVictimDirection()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[TCHAR_5]] - code
- [[UMVHitResolverSubsystemBuildResolvedHitData()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[UMVHitResolverSubsystemResolveAttackHit()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[UMVHitResolverSubsystemResolveHitDirection()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp
- [[UMVHitResolverSubsystemResolveNonNegativeStat()]] - code - Source/Maverick/Combat/MVHitResolverSubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_PIE 전투 디버그 도구]]
- 2 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 1 edge to [[_COMMUNITY_어빌리티 피격 Launch]]
- 1 edge to [[_COMMUNITY_PIE 피격 판정 테스트]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]

## Top bridge nodes
- [[MVHitResolverSubsystem.cpp]] - degree 11, connects to 2 communities
- [[FMVHitResolveRequest()]] - degree 8, connects to 2 communities
- [[MVHitResolverTryResolveAttackerToVictimDirection()]] - degree 5, connects to 1 community
- [[UMVHitResolverSubsystemResolveAttackHit()]] - degree 5, connects to 1 community
- [[UMVHitResolverSubsystemResolveHitDirection()]] - degree 5, connects to 1 community