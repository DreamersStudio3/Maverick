---
type: community
cohesion: 0.12
members: 21
---

# 피격 반응 Row 조회

**Cohesion:** 0.12 - loosely connected
**Members:** 21 nodes

## Members
- [[ActionRow_2]] - code - Source/Maverick/Components/MVHitReactionComponent.h
- [[ActionRowHandle]] - code - Source/Maverick/Components/MVHitReactionComponent.h
- [[Direction]] - code - Source/Maverick/Components/MVHitReactionComponent.h
- [[EMVHitReactionDirection_1]] - code
- [[FDataTableRowHandle_10]] - code
- [[FDataTableRowHandle_11]] - code
- [[FMVActionRow_1]] - code
- [[FMVHitReactionActionData]] - code - Source/Maverick/Components/MVHitReactionComponent.h
- [[FMVHitReactionActionRow()]] - code - Source/Maverick/Public/Tables/MVHitReactionActionTableTypes.h
- [[FName_40]] - code
- [[MAVERICK_API_12]] - code
- [[MVHitReactionBuildAvailableRowNameLog()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[MVHitReactionCopyBaseActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[StartSection_4]] - code - Source/Maverick/Components/MVHitReactionComponent.h
- [[UDataTable_6]] - code
- [[UMVHitReactionComponentFindBaseActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentFindHitReactionActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentFindRecoveryActionRow()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentGetActionData()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentResolveRecoveryActionRowHandle()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp
- [[UMVHitReactionComponentTryStartRecoveryAction()]] - code - Source/Maverick/Components/MVHitReactionComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__Row_
SORT file.name ASC
```

## Connections to other communities
- 8 edges to [[_COMMUNITY_피격 반응 상태 관리]]
- 3 edges to [[_COMMUNITY_피격 회복 방향 추적]]
- 2 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 2 edges to [[_COMMUNITY_액션 행·몽타주 타입]]
- 1 edge to [[_COMMUNITY_피격 리액션 입력·행 해석]]

## Top bridge nodes
- [[MVHitReactionBuildAvailableRowNameLog()]] - degree 7, connects to 2 communities
- [[UMVHitReactionComponentGetActionData()]] - degree 5, connects to 2 communities
- [[UMVHitReactionComponentResolveRecoveryActionRowHandle()]] - degree 4, connects to 2 communities
- [[UMVHitReactionComponentTryStartRecoveryAction()]] - degree 3, connects to 2 communities
- [[FMVHitReactionActionData]] - degree 10, connects to 1 community