---
type: community
cohesion: 0.11
members: 18
---

# 테이블 Row 공통 타입

**Cohesion:** 0.11 - loosely connected
**Members:** 18 nodes

## Members
- [[FMVCharacterIndexRow()]] - code - Source/Maverick/Public/Tables/MVCharacterTableTypes.h
- [[FMVCharacterStatRow()]] - code - Source/Maverick/Public/Tables/MVStatTableTypes.h
- [[FMVGameGuideRow()]] - code - Source/Maverick/Public/Tables/MVGameGuideTableTypes.h
- [[FMVGenericTableRow()]] - code - Source/Maverick/Public/Tables/MVTableTypes.h
- [[FMVTableManifestRow()]] - code - Source/Maverick/Public/Tables/MVTableTypes.h
- [[FMVTableRowBase()_1]] - code - Source/Maverick/Public/Tables/MVDialogueTableTypes.h
- [[FMVTableRowBase()_2]] - code - Source/Maverick/Public/Tables/MVTableTypes.h
- [[FMVUIMessageRow()]] - code - Source/Maverick/Public/Tables/MVUIMessageTableTypes.h
- [[MVCharacterTableTypes.h]] - code - Source/Maverick/Public/Tables/MVCharacterTableTypes.h
- [[MVDialogueTableTypes.h]] - code - Source/Maverick/Public/Tables/MVDialogueTableTypes.h
- [[MVGameGuideTableTypes.h]] - code - Source/Maverick/Public/Tables/MVGameGuideTableTypes.h
- [[MVStatTableTypes.h]] - code - Source/Maverick/Public/Tables/MVStatTableTypes.h
- [[MVTableManager.h]] - code - Source/Maverick/Public/Tables/MVTableManager.h
- [[MVTableTypes.h]] - code - Source/Maverick/Public/Tables/MVTableTypes.h
- [[MVUIMessageTableTypes.h]] - code - Source/Maverick/Public/Tables/MVUIMessageTableTypes.h
- [[UDataTable_10]] - code - Source/Maverick/Public/Tables/MVTableManager.h
- [[UEngineSubsystem()]] - code - Source/Maverick/Public/Tables/MVTableManager.h
- [[USoundBase]] - code - Source/Maverick/Public/Tables/MVDialogueTableTypes.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/_Row__
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 1 edge to [[_COMMUNITY_공통 캐릭터 선언]]
- 1 edge to [[_COMMUNITY_피격 반응 상태 관리]]
- 1 edge to [[_COMMUNITY_캐릭터 스탯 관리]]
- 1 edge to [[_COMMUNITY_시트 레시피 사양 로더]]
- 1 edge to [[_COMMUNITY_DataTable 에셋 생성]]
- 1 edge to [[_COMMUNITY_액션 행·몽타주 타입]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]
- 1 edge to [[_COMMUNITY_플레이어 이동 액션 테이블]]
- 1 edge to [[_COMMUNITY_테이블 후처리 인터페이스]]
- 1 edge to [[_COMMUNITY_런타임 PropTable 조회]]

## Top bridge nodes
- [[MVTableTypes.h]] - degree 16, connects to 7 communities
- [[MVCharacterTableTypes.h]] - degree 4, connects to 2 communities
- [[MVDialogueTableTypes.h]] - degree 4, connects to 1 community
- [[MVStatTableTypes.h]] - degree 3, connects to 1 community
- [[MVUIMessageTableTypes.h]] - degree 3, connects to 1 community