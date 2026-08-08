---
type: community
cohesion: 0.13
members: 15
---

# 상호작용 프롬프트 UI

**Cohesion:** 0.13 - loosely connected
**Members:** 15 nodes

## Members
- [[CandidateCount]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[CandidateIndex]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[FKey]] - code
- [[FMVInteractionPromptData]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[FText_9]] - code
- [[InputKey]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[MVInteractionPromptPopup.cpp]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp
- [[PromptText]] - code - Source/Maverick/UI/System/MVUIDataTypes.h
- [[UMVInteractionPromptPopup]] - code - Source/Maverick/UI/System/MVUISubsystem.h
- [[UMVInteractionPromptPopupRefreshPromptWidgets()]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp
- [[UMVInteractionPromptPopupSetPromptData()]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp
- [[UMVInteractionPromptPopupSetPromptText()]] - code - Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp
- [[UMVUISubsystemShowInteractionPrompt()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemShowInteractionPromptText()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[int32_35]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__UI
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_UI 팝업 메시지 데이터]]
- 2 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 1 edge to [[_COMMUNITY_필드 전환 리셋 계약]]
- 1 edge to [[_COMMUNITY_대화창 표시 수명주기]]
- 1 edge to [[_COMMUNITY_UI 서브시스템 공용 선언]]

## Top bridge nodes
- [[UMVInteractionPromptPopup]] - degree 4, connects to 2 communities
- [[UMVUISubsystemShowInteractionPromptText()]] - degree 3, connects to 2 communities
- [[FMVInteractionPromptData]] - degree 10, connects to 1 community
- [[UMVUISubsystemShowInteractionPrompt()]] - degree 3, connects to 1 community