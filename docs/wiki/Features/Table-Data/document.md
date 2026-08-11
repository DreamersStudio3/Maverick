---
제목: 테이블 데이터
부제목: CSV·DataTable·manifest·런타임 조회 파이프라인
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---
# 테이블 데이터

```text
MaverickDesign/Csv
  -> CsvToJsonConverter.py
  -> MaverickDesign/Json + SheetRecipe
  -> editor-only UMVTableAssetGenerator
  -> generated /Game/Table/DT_*
                                  -> DT_MVTableManifest
direct-managed /Game/Table/DT_*  -> DT_MVTableManifest
  -> UMVTableManager
  -> typed FindRow / TMVPropTable / Blueprint lookup
```

JSON은 에디터 중간 산출물이고 런타임 기준 데이터는 CSV에서 생성한 테이블, 직접 관리하는 `UDataTable`, manifest다. `UMVTableManager`는 `UEngineSubsystem`으로 typed row와 Blueprint reflection 조회를 제공한다. 현재 직접 관리 root는 `Attack`, `Death`, `Dodge`, `Groggy`, `HitReaction`, `Props`, `Sprint`, `Weapons`이며 `MaverickDesign/README.md`의 목록과 drift가 있다. 실제 행, row struct, source hash와 manifest 완전성은 에디터 검증이 필요하다.

`GameGuide`는 도움말과 팁의 노출 문구만 소유한다. 아이템 설명이나 로어를 복제하지 않으며, 추후 아이템 정보를 도움말로 보여줄 때는 별도 큐레이션 데이터가 `Item` row를 참조한다.
