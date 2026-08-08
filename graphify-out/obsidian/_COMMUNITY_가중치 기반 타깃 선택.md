---
type: community
cohesion: 0.28
members: 9
---

# 가중치 기반 타깃 선택

**Cohesion:** 0.28 - loosely connected
**Members:** 9 nodes

## Members
- [[CreateFindTargetContext]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/WeightedTargetHandler.h
- [[FFindTargetRequestParams]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/LockOnTargetComponent.h
- [[FFindTargetRequestResponse()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/TargetHandlerBase.h
- [[FindTargetBatched]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Public/TargetHandlers/WeightedTargetHandler.h
- [[ULockOnTargetComponentProcessTargetHandlerResponse()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[ULockOnTargetComponentRequestFindTarget()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/LockOnTargetComponent.cpp
- [[UTargetHandlerBaseFindTarget_Implementation()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetHandlers/TargetHandlerBase.cpp
- [[UWeightedTargetHandlerFindTarget_Implementation()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetHandlers/WeightedTargetHandler.cpp
- [[UWeightedTargetHandlerTryFindTarget()]] - code - Plugins/LockOnTarget/Source/LockOnTarget/Private/TargetHandlers/WeightedTargetHandler.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_가중 대상 탐색 컨텍스트]]
- 3 edges to [[_COMMUNITY_가중 대상 탐색 알고리즘]]
- 2 edges to [[_COMMUNITY_락온 대상 컴포넌트]]
- 1 edge to [[_COMMUNITY_락온 디버거]]
- 1 edge to [[_COMMUNITY_LockOnTarget 모듈·타깃 핸들러]]
- 1 edge to [[_COMMUNITY_락온 핵심 타입과 핸들러]]
- 1 edge to [[_COMMUNITY_LockOnTarget 서브오브젝트 수명주기]]

## Top bridge nodes
- [[FFindTargetRequestResponse()]] - degree 7, connects to 4 communities
- [[FFindTargetRequestParams]] - degree 5, connects to 2 communities
- [[UWeightedTargetHandlerFindTarget_Implementation()]] - degree 5, connects to 1 community
- [[UTargetHandlerBaseFindTarget_Implementation()]] - degree 3, connects to 1 community
- [[UWeightedTargetHandlerTryFindTarget()]] - degree 3, connects to 1 community