---
type: community
cohesion: 0.20
members: 14
---

# 사망 액션 행 이름 해석

**Cohesion:** 0.20 - loosely connected
**Members:** 14 nodes

## Members
- [[EMVDeathActionFacing]] - code
- [[FDataTableRowHandle_8]] - code
- [[FGameplayTag_7]] - code
- [[FName_37]] - code
- [[UMVDeathComponentCharacterIndexCodeToTableToken()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentHandleActionEnded()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentMakeDeathActionRowName()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentMakeDeathActionTableName()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentMakeLegacyDeathActionRowName()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentResolveDeathActionFacing()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentResolveDeathActionRowHandle()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentResolveDeathActionTableName()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[UMVDeathComponentTryBeginDeferredDeathPresentation()]] - code - Source/Maverick/Components/MVDeathComponent.cpp
- [[int32_17]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/____
SORT file.name ASC
```

## Connections to other communities
- 9 edges to [[_COMMUNITY_캐릭터 사망 표현]]
- 3 edges to [[_COMMUNITY_사망 액션 포즈 해석]]
- 2 edges to [[_COMMUNITY_사망 표현 보류·시작]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]

## Top bridge nodes
- [[UMVDeathComponentResolveDeathActionRowHandle()]] - degree 6, connects to 3 communities
- [[UMVDeathComponentMakeDeathActionRowName()]] - degree 6, connects to 2 communities
- [[UMVDeathComponentCharacterIndexCodeToTableToken()]] - degree 3, connects to 2 communities
- [[UMVDeathComponentResolveDeathActionFacing()]] - degree 3, connects to 2 communities
- [[UMVDeathComponentMakeLegacyDeathActionRowName()]] - degree 5, connects to 1 community