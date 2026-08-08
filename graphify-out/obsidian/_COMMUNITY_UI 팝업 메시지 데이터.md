---
type: community
cohesion: 0.10
members: 23
---

# UI 팝업 메시지 데이터

**Cohesion:** 0.10 - loosely connected
**Members:** 23 nodes

## Members
- [[ActionName]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[Duration]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[FMVMenuEntryData]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[FMVPopupMessageData]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[FName_49]] - code
- [[FText_10]] - code
- [[FText_11]] - code
- [[Label_1]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[MVInteractionPromptPopup.h]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.h
- [[MVMessagePopup.cpp]] - code - Source/Maverick/UI/Popup/MVMessagePopup.cpp
- [[MVMessagePopup.h]] - code - Source/Maverick/UI/Popup/MVMessagePopup.h
- [[MVUIDataTypes.h]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[MessageId]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[MessageText]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[UMVInteractionPromptPopup()]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.h
- [[UMVMessagePopup()]] - code - Source/Maverick/UI/Popup/MVMessagePopup.h
- [[UMVMessagePopupSetMessageData()]] - code - Source/Maverick/UI/Popup/MVMessagePopup.cpp
- [[UMVMessagePopupSetMessageText()]] - code - Source/Maverick/UI/Popup/MVMessagePopup.cpp
- [[UMVUISubsystemShowPopupMessage()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemShowPopupMessageText()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UTextBlock_7]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.h
- [[UTextBlock_8]] - code - Source/Maverick/UI/Popup/MVMessagePopup.h
- [[bEnabled]] - code - Source/Maverick/UI/System/MVUIDataTypes.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/UI___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_상호작용 프롬프트 UI]]
- 2 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 1 edge to [[_COMMUNITY_플레이어 상호작용 감지]]
- 1 edge to [[_COMMUNITY_테이블 기반 UI·스탯 조회]]
- 1 edge to [[_COMMUNITY_UI 서브시스템 공용 선언]]
- 1 edge to [[_COMMUNITY_대화창 표시 수명주기]]

## Top bridge nodes
- [[MVUIDataTypes.h]] - degree 7, connects to 3 communities
- [[UMVUISubsystemShowPopupMessageText()]] - degree 3, connects to 2 communities
- [[UMVMessagePopup()]] - degree 4, connects to 1 community
- [[FText_11]] - degree 3, connects to 1 community
- [[UMVUISubsystemShowPopupMessage()]] - degree 3, connects to 1 community