---
type: community
cohesion: 0.22
members: 10
---

# LockOnTarget 서브오브젝트 수명주기

**Cohesion:** 0.22 - loosely connected
**Members:** 10 nodes

## Members
- [[FFindTargetRequestParams()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h
- [[FLifetimeProperty]] - code
- [[TArray]] - code
- [[TInlineAllocator]] - code
- [[TargetHandlerBase.h]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h
- [[ULockOnTargetComponentDestroySubobject()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[ULockOnTargetComponentGetAllSubobjects()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[ULockOnTargetComponentGetLifetimeReplicatedProps()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[ULockOnTargetComponentInitializeSubobject()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[ULockOnTargetExtensionProxy()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/LockOnTarget__
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_락온 대상 컴포넌트]]
- 1 edge to [[_COMMUNITY_락온 핵심 타입과 핸들러]]
- 1 edge to [[_COMMUNITY_가중치 기반 타깃 선택]]

## Top bridge nodes
- [[ULockOnTargetExtensionProxy()]] - degree 6, connects to 1 community
- [[ULockOnTargetComponentGetAllSubobjects()]] - degree 4, connects to 1 community
- [[ULockOnTargetComponentGetLifetimeReplicatedProps()]] - degree 3, connects to 1 community
- [[TargetHandlerBase.h]] - degree 3, connects to 1 community
- [[ULockOnTargetComponentDestroySubobject()]] - degree 2, connects to 1 community