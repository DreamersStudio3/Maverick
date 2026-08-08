---
type: community
members: 12
---

# 테이블 에셋 매니페스트 관리

**Members:** 12 nodes

## Members
- [[CreateOrResetDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[FindExistingDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[SaveDataTableAsset()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorCanReuseDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorReadStoredSourceHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorResolveKeyColumnName()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorWriteStoredSourceHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UDataTable_8]] - code
- [[UMVTableAssetGeneratorBuildManifestRowForDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorSaveManifest()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UObject_10]] - code
- [[UScriptStruct_1]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 18 edges to [[_COMMUNITY_DataTable 에셋 생성]]
- 5 edges to [[_COMMUNITY_JSON 테이블 생성·해시]]
- 3 edges to [[_COMMUNITY_테이블 매니페스트 생성과 검증]]
- 1 edge to [[_COMMUNITY_JSON 스키마 값 타입]]

## Top bridge nodes
- [[UMVTableAssetGeneratorSaveManifest()]] - degree 10, connects to 3 communities
- [[TableAssetGeneratorCanReuseDataTable()]] - degree 7, connects to 2 communities
- [[CreateOrResetDataTable()]] - degree 6, connects to 2 communities
- [[TableAssetGeneratorWriteStoredSourceHash()]] - degree 5, connects to 2 communities
- [[SaveDataTableAsset()]] - degree 5, connects to 2 communities