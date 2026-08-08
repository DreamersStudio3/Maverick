---
type: community
members: 6
---

# 테이블 기반 UI·스탯 조회

**Members:** 6 nodes

## Members
- [[FName_50]] - code
- [[UMVLoadingWindowLoadLoadingGuideCards()]] - code - Source/Maverick/UI/Window/MVLoadingWindow.cpp
- [[UMVStatComponentLoadStatsFromTable()]] - code - Source/Maverick/Components/MVStatComponent.cpp
- [[UMVTableManager]] - code - Source/Maverick/Components/MVStatComponent.h
- [[UMVUISubsystemShowDialogueWindowById()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemShowPopupMessageById()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__UI_
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_DataTable 에셋 생성]]
- 2 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 1 edge to [[_COMMUNITY_회피 입력 컨텍스트]]
- 1 edge to [[_COMMUNITY_회피·질주 액션 데이터]]
- 1 edge to [[_COMMUNITY_액션 Row 몽타주 해석]]
- 1 edge to [[_COMMUNITY_사망 액션 행 이름 해석]]
- 1 edge to [[_COMMUNITY_피격 반응 행 조회]]
- 1 edge to [[_COMMUNITY_캐릭터 스탯 관리]]
- 1 edge to [[_COMMUNITY_회복 스탯 일시정지 노티파이]]
- 1 edge to [[_COMMUNITY_런타임 테이블 조회]]
- 1 edge to [[_COMMUNITY_UI 팝업 메시지 데이터]]
- 1 edge to [[_COMMUNITY_대화창 표시 대기열 수명주기]]
- 1 edge to [[_COMMUNITY_로딩 화면 입력 처리]]

## Top bridge nodes
- [[UMVTableManager]] - degree 13, connects to 8 communities
- [[UMVUISubsystemShowDialogueWindowById()]] - degree 4, connects to 2 communities
- [[UMVUISubsystemShowPopupMessageById()]] - degree 4, connects to 2 communities
- [[UMVStatComponentLoadStatsFromTable()]] - degree 2, connects to 1 community
- [[UMVLoadingWindowLoadLoadingGuideCards()]] - degree 2, connects to 1 community