# 상호작용 프롬프트 UI

> 15 nodes · cohesion 0.13

## Key Concepts

- **FMVInteractionPromptData** (10 connections) — `Source/Maverick/UI/System/MVUIDataTypes.h`
- **UMVInteractionPromptPopup** (4 connections) — `Source/Maverick/UI/System/MVUISubsystem.h`
- **MVInteractionPromptPopup.cpp** (3 connections) — `Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp`
- **UMVUISubsystem::ShowInteractionPrompt()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::ShowInteractionPromptText()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVInteractionPromptPopup::SetPromptData()** (2 connections) — `Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp`
- **UMVInteractionPromptPopup::SetPromptText()** (2 connections) — `Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp`
- **FText** (1 connections)
- **UMVInteractionPromptPopup::RefreshPromptWidgets()** (1 connections) — `Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp`
- **CandidateCount** (1 connections) — `Source/Maverick/UI/System/MVUIDataTypes.h`
- **CandidateIndex** (1 connections) — `Source/Maverick/UI/System/MVUIDataTypes.h`
- **InputKey** (1 connections) — `Source/Maverick/UI/System/MVUIDataTypes.h`
- **PromptText** (1 connections) — `Source/Maverick/UI/System/MVUIDataTypes.h`
- **FKey** (1 connections)
- **int32** (1 connections)

## Relationships

- [UI 팝업 메시지 데이터](UI_%ED%8C%9D%EC%97%85_%EB%A9%94%EC%8B%9C%EC%A7%80_%EB%8D%B0%EC%9D%B4%ED%84%B0.md) (2 shared connections)
- [UI 서브시스템 팝업 관리](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%ED%8C%9D%EC%97%85_%EA%B4%80%EB%A6%AC.md) (2 shared connections)
- [필드 전환 리셋 계약](%ED%95%84%EB%93%9C_%EC%A0%84%ED%99%98_%EB%A6%AC%EC%85%8B_%EA%B3%84%EC%95%BD.md) (1 shared connections)
- [UI 서브시스템 공용 선언](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%EC%84%A0%EC%96%B8.md) (1 shared connections)
- [대화창 표시 수명주기](%EB%8C%80%ED%99%94%EC%B0%BD_%ED%91%9C%EC%8B%9C_%EC%88%98%EB%AA%85%EC%A3%BC%EA%B8%B0.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/Popup/MVInteractionPromptPopup.cpp`
- `Source/Maverick/UI/System/MVUIDataTypes.h`
- `Source/Maverick/UI/System/MVUISubsystem.cpp`
- `Source/Maverick/UI/System/MVUISubsystem.h`

## Audit Trail

- EXTRACTED: 35 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*