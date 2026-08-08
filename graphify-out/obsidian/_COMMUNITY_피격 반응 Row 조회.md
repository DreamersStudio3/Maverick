---
type: community
cohesion: 0.16
members: 17
---

# 피격 반응 Row 조회

**Cohesion:** 0.16 - loosely connected
**Members:** 17 nodes

## Members
- [[FDataTableRowHandle_10]] - code
- [[FMVActionRow_1]] - code
- [[FMVHitReactionActionRow()]] - code - Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h
- [[FMVHitReactionActionRowHandle()]] - code - Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h
- [[MAVERICK_API_12]] - code
- [[MVHitReactionActionTableTypes.h]] - code - Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h
- [[MVHitReactionBuildAvailableRowNameLog()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionCopyBaseActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UDataTable_6]] - code
- [[UMVHitReactionComponentFindBaseActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentFindHitReactionActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentFindRecoveryActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentGetActionData()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentMakeHitReactionActionRowHandleFromNames()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveHitReactionActionRowHandle()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveRecoveryActionRowHandle()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryStartRecoveryAction()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__Row_
SORT file.name ASC
```

## Connections to other communities
- 10 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 4 edges to [[_COMMUNITY_피격 회복 방향 추적]]
- 2 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 2 edges to [[_COMMUNITY_피격 리액션 입력·행 해석]]
- 2 edges to [[_COMMUNITY_피격 리액션 액션 데이터]]
- 1 edge to [[_COMMUNITY_전투 액션 Chooser 입력]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]
- 1 edge to [[_COMMUNITY_시트 레시피 사양 로더]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]

## Top bridge nodes
- [[UMVHitReactionComponentGetActionData()]] - degree 5, connects to 3 communities
- [[MVHitReactionActionTableTypes.h]] - degree 5, connects to 3 communities
- [[UMVHitReactionComponentMakeHitReactionActionRowHandleFromNames()]] - degree 4, connects to 3 communities
- [[MVHitReactionBuildAvailableRowNameLog()]] - degree 7, connects to 2 communities
- [[UMVHitReactionComponentResolveHitReactionActionRowHandle()]] - degree 4, connects to 2 communities