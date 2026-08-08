# UI 레이어 오버레이 구성

> 8 nodes

## Key Concepts

- **UOverlay** (7 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **MVUILayerBase.h** (5 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVUILayerBase::BuildNativeLayerTree()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVUILayerBase::AddFullScreenOverlayChild()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.cpp`
- **UMVDeathOverlayWindow::BuildNativeWidgetTree()** (3 connections) — `Source/Maverick/UI/Window/MVDeathOverlayWindow.cpp`
- **UCommonActivatableWidgetStack** (2 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UCommonActivatableWidget** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVPopupBase** (1 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`

## Relationships

- [UI 레이어와 사망 오버레이](UI_%EB%A0%88%EC%9D%B4%EC%96%B4%EC%99%80_%EC%82%AC%EB%A7%9D_%EC%98%A4%EB%B2%84%EB%A0%88%EC%9D%B4.md) (2 shared connections)
- [스킬 트리 UI 탐색](%EC%8A%A4%ED%82%AC_%ED%8A%B8%EB%A6%AC_UI_%ED%83%90%EC%83%89.md) (1 shared connections)
- [로딩 UI 테스트 명령](%EB%A1%9C%EB%94%A9_UI_%ED%85%8C%EC%8A%A4%ED%8A%B8_%EB%AA%85%EB%A0%B9.md) (1 shared connections)
- [스킬 슬롯 위젯 구성](%EC%8A%A4%ED%82%AC_%EC%8A%AC%EB%A1%AF_%EC%9C%84%EC%A0%AF_%EA%B5%AC%EC%84%B1.md) (1 shared connections)
- [상태 바 위젯](%EC%83%81%ED%83%9C_%EB%B0%94_%EC%9C%84%EC%A0%AF.md) (1 shared connections)
- [로딩 창 구성과 진행률](%EB%A1%9C%EB%94%A9_%EC%B0%BD_%EA%B5%AC%EC%84%B1%EA%B3%BC_%EC%A7%84%ED%96%89%EB%A5%A0.md) (1 shared connections)
- [사망 오버레이 페이드](%EC%82%AC%EB%A7%9D_%EC%98%A4%EB%B2%84%EB%A0%88%EC%9D%B4_%ED%8E%98%EC%9D%B4%EB%93%9C.md) (1 shared connections)
- [PIE 액션 테스트 UI](PIE_%EC%95%A1%EC%85%98_%ED%85%8C%EC%8A%A4%ED%8A%B8_UI.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/System/MVUILayerBase.cpp`
- `Source/Maverick/UI/System/MVUILayerBase.h`
- `Source/Maverick/UI/Window/MVDeathOverlayWindow.cpp`

## Audit Trail

- EXTRACTED: 24 (96%)
- INFERRED: 1 (4%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*