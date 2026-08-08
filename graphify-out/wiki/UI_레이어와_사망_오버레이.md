# UI 레이어와 사망 오버레이

> 22 nodes · cohesion 0.12

## Key Concepts

- **MVUILayerBase.cpp** (10 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UOverlay** (7 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **MVUILayerBase.h** (5 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVUILayerBase::SetHUDByClass()** (4 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **TSubclassOf** (3 connections)
- **UUserWidget** (3 connections)
- **UMVUILayerBase::AddFullScreenOverlayChild()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::BuildNativeLayerTree()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::PushPopup()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::PushWindow()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVDeathOverlayWindow::BuildNativeWidgetTree()** (3 connections) — `Source/Maverick/UI/Window/MVDeathOverlayWindow.cpp`
- **UCommonActivatableWidgetStack** (2 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVUILayerBase::AddWidgetLayerWidget()** (2 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::SetHUDWidget()** (2 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **APlayerController** (1 connections)
- **UCommonActivatableWidget** (1 connections)
- **UMVPopupBase** (1 connections)
- **UCommonActivatableWidget** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVPopupBase** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVUILayerBase::ClearLayer()** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::HideHUDWidgetWithFade()** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::NativeOnInitialized()** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`

## Relationships

- [로딩 UI 테스트 명령](%EB%A1%9C%EB%94%A9_UI_%ED%85%8C%EC%8A%A4%ED%8A%B8_%EB%AA%85%EB%A0%B9.md) (1 shared connections)
- [UI 기반 전환 유틸리티](UI_%EA%B8%B0%EB%B0%98_%EC%A0%84%ED%99%98_%EC%9C%A0%ED%8B%B8%EB%A6%AC%ED%8B%B0.md) (1 shared connections)
- [스킬 슬롯 위젯 구성](%EC%8A%A4%ED%82%AC_%EC%8A%AC%EB%A1%AF_%EC%9C%84%EC%A0%AF_%EA%B5%AC%EC%84%B1.md) (1 shared connections)
- [상태 바 위젯](%EC%83%81%ED%83%9C_%EB%B0%94_%EC%9C%84%EC%A0%AF.md) (1 shared connections)
- [로딩 화면 입력 처리](%EB%A1%9C%EB%94%A9_%ED%99%94%EB%A9%B4_%EC%9E%85%EB%A0%A5_%EC%B2%98%EB%A6%AC.md) (1 shared connections)
- [사망 오버레이 페이드](%EC%82%AC%EB%A7%9D_%EC%98%A4%EB%B2%84%EB%A0%88%EC%9D%B4_%ED%8E%98%EC%9D%B4%EB%93%9C.md) (1 shared connections)
- [PIE 액션 테스트 UI](PIE_%EC%95%A1%EC%85%98_%ED%85%8C%EC%8A%A4%ED%8A%B8_UI.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/System/MVUILayerBase.cpp`
- `Source/Maverick/UI/System/MVUILayerBase.h`
- `Source/Maverick/UI/Window/MVDeathOverlayWindow.cpp`

## Audit Trail

- EXTRACTED: 60 (98%)
- INFERRED: 1 (2%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*