---
type: community
cohesion: 0.22
members: 9
---

# LockOnTarget 타깃 캡처 상태

**Cohesion:** 0.22 - loosely connected
**Members:** 9 nodes

## Members
- [[dot-GetDefaultSocket()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[dot-GetInvadersNum()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[dot-IsCaptured()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[dot-IsSocketValid()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[FName_12]] - code
- [[K2_OnCaptured]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetComponent.h
- [[UTargetComponentDispatchTargetException()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp
- [[UTargetComponentNotifyTargetCaptured()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetComponent.cpp
- [[int32_1]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/LockOnTarget___
SORT file.name ASC
```

## Connections to other communities
- 6 edges to [[_COMMUNITY_락온 대상 포커스 설정]]
- 2 edges to [[_COMMUNITY_락온 대상 소켓 관리]]
- 1 edge to [[_COMMUNITY_LockOnTarget 포커스 지점 해석]]
- 1 edge to [[_COMMUNITY_락온 대상 예외 처리]]

## Top bridge nodes
- [[UTargetComponentDispatchTargetException()]] - degree 4, connects to 2 communities
- [[UTargetComponentNotifyTargetCaptured()]] - degree 4, connects to 2 communities
- [[dot-IsSocketValid()]] - degree 4, connects to 1 community
- [[dot-GetInvadersNum()]] - degree 3, connects to 1 community
- [[dot-IsCaptured()]] - degree 3, connects to 1 community