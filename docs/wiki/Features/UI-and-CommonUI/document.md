---
제목: UI와 CommonUI
부제목: UI 계층과 사망·필드 전환 화면 오케스트레이션
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---
# UI와 CommonUI

`UMVUISubsystem`은 GameInstance 수명의 UI 진입점이다.

```text
Viewport
  -> UMVUILayerBase
       -> WindowStack : CommonActivatableWidgetStack
       -> HUDLayer    : Overlay
       -> PopupLayer  : Overlay
       -> WidgetLayer : Overlay
```

- Window만 CommonUI activatable stack과 modal/back/focus/input lifecycle에 참여한다.
- Popup과 HUD는 단일 overlay이며 stack activation을 소유하지 않는다.
- DeathRespawnFlow가 death overlay를 표시한다. FieldTransition은 loading window와 전환 전후 `ClearAllUI`, `ResetToDefaultUI`를 사용해 화면 전환을 오케스트레이션한다.
- WBP 내부 widget tree, animation, Blueprint binding은 `에셋 확인 필요`다.
