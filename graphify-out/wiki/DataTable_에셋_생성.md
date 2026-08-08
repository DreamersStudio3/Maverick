# DataTable 에셋 생성

> 64 nodes · cohesion 0.11

## Key Concepts

- **MVTableAssetGenerator.cpp** (47 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **FString** (44 connections)
- **UMVTableAssetGenerator::BuildAndSaveDataTable()** (17 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TArray** (16 connections)
- **FMVTableManifestRow** (15 connections) — `Source/Maverick/Public/Tables/MVTableAssetGenerator.h`
- **TableAssetGeneratorBuildGeneratedTableHash()** (10 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **UMVTableAssetGenerator::BuildCsvOriginManifestRowsFromJsonFile()** (10 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **UMVTableAssetGenerator::SaveManifest()** (10 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TryGetArrayField()** (9 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **UMVTableAssetGenerator::ImportJsonFile()** (9 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TSharedPtr** (8 connections)
- **JsonValueToGeneratorString()** (8 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorReadStringKeySet()** (8 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TryGetObjectField()** (8 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **FJsonValue** (8 connections) — `Source/Maverick/Public/Tables/MVTableAssetGenerator.h`
- **GetJsonDir()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorAddManifestRow()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorBuildManifestHash()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorCanReuseDataTable()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorReadIntegerKeySet()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **UMVTableAssetGenerator::BuildCsvOriginManifestRows()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **UMVTableAssetGenerator::ImportAllJsonFiles()** (7 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **CreateOrResetDataTable()** (6 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **GetConverterDir()** (6 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- **TableAssetGeneratorAppendJsonHashValue()** (6 connections) — `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- *... and 39 more nodes in this community*

## Relationships

- [런타임 테이블 조회](%EB%9F%B0%ED%83%80%EC%9E%84_%ED%85%8C%EC%9D%B4%EB%B8%94_%EC%A1%B0%ED%9A%8C.md) (4 shared connections)
- [테이블 기반 UI·스탯 조회](%ED%85%8C%EC%9D%B4%EB%B8%94_%EA%B8%B0%EB%B0%98_UI%C2%B7%EC%8A%A4%ED%83%AF_%EC%A1%B0%ED%9A%8C.md) (2 shared connections)
- [테이블 JSON 필드 제약](%ED%85%8C%EC%9D%B4%EB%B8%94_JSON_%ED%95%84%EB%93%9C_%EC%A0%9C%EC%95%BD.md) (1 shared connections)
- [JSON 스키마 값 타입](JSON_%EC%8A%A4%ED%82%A4%EB%A7%88_%EA%B0%92_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [액션 행·몽타주 타입](%EC%95%A1%EC%85%98_%ED%96%89%C2%B7%EB%AA%BD%ED%83%80%EC%A3%BC_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [테이블 Row 공통 타입](%ED%85%8C%EC%9D%B4%EB%B8%94_Row_%EA%B3%B5%ED%86%B5_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [테이블 JSON 스키마](%ED%85%8C%EC%9D%B4%EB%B8%94_JSON_%EC%8A%A4%ED%82%A4%EB%A7%88.md) (1 shared connections)

## Source Files

- `MaverickDesign/Schema/TableData.schema.json`
- `Source/Maverick/Private/Tables/MVTableAssetGenerator.cpp`
- `Source/Maverick/Public/Tables/MVTableAssetGenerator.h`

## Audit Trail

- EXTRACTED: 451 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*