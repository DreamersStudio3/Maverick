# 플레이어 상태 HUD 연동

> 16 nodes

## Key Concepts

- **MVPlayerStatusWidget.cpp** (12 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **MVStatusBarWidget.h** (3 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`
- **UMVPlayerStatusWidget::ApplyStatusBarSize()** (2 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVStatusBarWidget()** (2 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`
- **UMVPlayerStatusWidget::UnbindStatComponent()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::NativeOnInitialized()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::NativeConstruct()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::NativeDestruct()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::BuildNativeWidgetTree()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::ConfigureStatusBars()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::HandleHPChanged()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::HandleStaminaChanged()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::HandleMPChanged()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::HandleStatRecentLossHoldChanged()** (1 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **USizeBox** (1 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`
- **UTextBlock** (1 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`

## Relationships

- [회복 스탯 일시정지 노티파이](%ED%9A%8C%EB%B3%B5_%EC%8A%A4%ED%83%AF_%EC%9D%BC%EC%8B%9C%EC%A0%95%EC%A7%80_%EB%85%B8%ED%8B%B0%ED%8C%8C%EC%9D%B4.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- `Source/Maverick/UI/HUD/MVStatusBarWidget.h`

## Audit Trail

- EXTRACTED: 31 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*