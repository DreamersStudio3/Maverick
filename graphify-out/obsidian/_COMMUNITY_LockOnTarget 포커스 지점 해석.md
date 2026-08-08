---
type: community
cohesion: 0.25
members: 9
---

# LockOnTarget 포커스 지점 해석

**Cohesion:** 0.25 - loosely connected
**Members:** 9 nodes

## Members
- [[FVector_3]] - code
- [[GetCustomFocusPoint]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[GetSocketLocation]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[K2_OnReleased]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[ULockOnTargetComponent_8]] - code
- [[UTargetComponentGetCustomFocusPoint_Implementation()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp
- [[UTargetComponentGetFocusPointLocation()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp
- [[UTargetComponentGetSocketLocation()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp
- [[UTargetComponentNotifyTargetReleased()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/LockOnTarget___
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_락온 대상 소켓 관리]]
- 3 edges to [[_COMMUNITY_락온 대상 포커스 설정]]
- 1 edge to [[_COMMUNITY_LockOnTarget 타깃 캡처 상태]]

## Top bridge nodes
- [[UTargetComponentGetFocusPointLocation()]] - degree 5, connects to 1 community
- [[ULockOnTargetComponent_8]] - degree 4, connects to 1 community
- [[UTargetComponentGetCustomFocusPoint_Implementation()]] - degree 3, connects to 1 community
- [[UTargetComponentGetSocketLocation()]] - degree 3, connects to 1 community
- [[UTargetComponentNotifyTargetReleased()]] - degree 3, connects to 1 community