---
제목: UI와 CommonUI
부제목: UI 계층과 사망·필드 전환 화면 오케스트레이션
최근수정일: 2026-08-12
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Features/Interaction-Flow/document|상호작용 흐름]]"
---

# UI와 CommonUI

`UMVUISubsystem`: GameInstance 수명의 UI 진입점

## 계층

```mermaid
flowchart TD
    Viewport["Viewport"] --> Layer["UMVUILayerBase"]
    Layer --> Window["WindowStack<br/>CommonActivatableWidgetStack"]
    Layer --> HUD["HUDLayer<br/>Overlay"]
    Layer --> Popup["PopupLayer<br/>Overlay"]
    Layer --> Widget["WidgetLayer<br/>Overlay"]
```

## 책임

| 계층 | 책임 |
|---|---|
| Window | CommonUI Activatable Stack, Modal, Back, Focus, Input 수명주기 |
| Popup·HUD | 단일 Overlay, Stack Activation 비소유 |
| WBP Blueprint | Widget Tree, Animation, 화면 Binding |

## 사망과 필드 전환

- DeathRespawnFlow: Death Overlay 표시
- FieldTransition: Loading Window와 화면 전환 오케스트레이션
- 전환 전후 `ClearAllUI`, `ResetToDefaultUI` 사용
