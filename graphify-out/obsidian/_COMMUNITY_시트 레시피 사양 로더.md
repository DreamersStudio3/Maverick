---
type: community
cohesion: 0.29
members: 10
---

# 시트 레시피 사양 로더

**Cohesion:** 0.29 - loosely connected
**Members:** 10 nodes

## Members
- [[FMVSheetSpec]] - code
- [[FMVSheetSpecsFind()]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[FMVSheetSpecsGetAllSheetNames()]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[FMVSheetSpecsInvalidate()]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[FString_20]] - code
- [[GetRecipePath()]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[LoadRecipe()]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[MVSheetSpecs.cpp]] - code - Source/Maverick/Private/Tables/MVSheetSpecs.cpp
- [[TArray_18]] - code
- [[TMap]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]
- 1 edge to [[_COMMUNITY_피격 반응 Row 조회]]
- 1 edge to [[_COMMUNITY_테이블 Row 공통 타입]]

## Top bridge nodes
- [[MVSheetSpecs.cpp]] - degree 8, connects to 3 communities