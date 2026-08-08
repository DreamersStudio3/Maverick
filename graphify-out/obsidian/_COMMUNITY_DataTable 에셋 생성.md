---
type: community
cohesion: 0.11
members: 64
---

# DataTable 에셋 생성

**Cohesion:** 0.11 - loosely connected
**Members:** 64 nodes

## Members
- [[CreateOrResetDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[FJsonObject]] - code
- [[FJsonValue]] - code - Source/Maverick/Public/Tables/MVTableAssetGenerator.h
- [[FMVSheetSpec_1]] - code
- [[FMVSheetSpec_3]] - code - Source/Maverick/Public/Tables/MVTableAssetGenerator.h
- [[FMVTableManifestRow]] - code - Source/Maverick/Public/Tables/MVTableAssetGenerator.h
- [[FString_21]] - code
- [[FindExistingDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[GetConverterDir()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[GetConverterExePath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[GetConverterScriptPath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[GetJsonDir()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[GetMaverickDesignDir()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[JsonObjectToCompactString()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[JsonValueToGeneratorString()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[MVTableAssetGenerator.cpp]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[MVTableAssetGenerator.h]] - code - Source/Maverick/Public/Tables/MVTableAssetGenerator.h
- [[SanitizeAssetName()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[SaveDataTableAsset()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TArray_19]] - code
- [[TSet]] - code
- [[TSharedPtr_3]] - code
- [[TableAssetGeneratorAddManifestRow()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorAppendJsonHashValue()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorBuildGeneratedTableHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorBuildManifestHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorCanReuseDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorHashSourceString()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorIsDirectManagedJsonPath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorIsDirectManagedObjectPath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorIsDirectManagedPackagePath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorIsUnderPackageRoot()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorReadIntegerKeySet()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorReadStoredSourceHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorReadStringKeySet()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorResolveKeyColumnName()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorSortManifestRows()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TableAssetGeneratorWriteStoredSourceHash()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[ToTableAssetName()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[ToTablePackagePath()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TryGetArrayField()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TryGetObjectField()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[TryGetStringField()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UBlueprintFunctionLibrary()]] - code - Source/Maverick/Public/Tables/MVTableAssetGenerator.h
- [[UDataTable_8]] - code
- [[UMVTableAssetGeneratorBuildAndSaveDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorBuildCsvOriginManifestRows()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorBuildCsvOriginManifestRowsFromJsonFile()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorBuildManifestRowForDataTable()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorDeleteStaleGeneratedAssets()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorGenerateDataTables()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorImportAllJsonFiles()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorImportJsonFile()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorRefreshTableManifest()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorRunCsvConverter()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorSaveManifest()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorScanDirectManagedDataTables()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UMVTableAssetGeneratorValidateCharacterStatMapping()]] - code - Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp
- [[UObject_10]] - code
- [[UScriptStruct_1]] - code
- [[additionalProperties]] - code - MaverickDesign/Schema/TableData.schema.json
- [[int32_23]] - code
- [[source]] - code - MaverickDesign/Schema/TableData.schema.json
- [[type_2]] - code - MaverickDesign/Schema/TableData.schema.json

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/DataTable__
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_런타임 테이블 조회]]
- 2 edges to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_테이블 JSON 필드 제약]]
- 1 edge to [[_COMMUNITY_JSON 스키마 값 타입]]
- 1 edge to [[_COMMUNITY_테이블 JSON 스키마]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]
- 1 edge to [[_COMMUNITY_테이블 Row 공통 타입]]

## Top bridge nodes
- [[MVTableAssetGenerator.cpp]] - degree 47, connects to 2 communities
- [[FMVTableManifestRow]] - degree 15, connects to 1 community
- [[FJsonValue]] - degree 8, connects to 1 community
- [[TableAssetGeneratorSortManifestRows()]] - degree 6, connects to 1 community
- [[source]] - degree 5, connects to 1 community