---
type: community
cohesion: 0.33
members: 13
---

# MVHitResolverSubsystem.cpp

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
TABLE source_file, type FROM #community/MVHitResolverSubsystemcpp
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_MVAbilityBase.cpp]]
- 2 edges to [[_COMMUNITY_MVHitTypes.h]]
- 2 edges to [[_COMMUNITY_FMVResolvedHitData]]
- 1 edge to [[_COMMUNITY_MVWeaponComponent.cpp]]

## Top bridge nodes
- [[MVHitResolverSubsystem.cpp]] - degree 11, connects to 2 communities
- [[FMVHitResolveRequest()]] - degree 8, connects to 2 communities
- [[MVHitResolverTryResolveAttackerToVictimDirection()]] - degree 5, connects to 1 community
- [[UMVHitResolverSubsystemResolveAttackHit()]] - degree 5, connects to 1 community
- [[UMVHitResolverSubsystemResolveHitDirection()]] - degree 5, connects to 1 community