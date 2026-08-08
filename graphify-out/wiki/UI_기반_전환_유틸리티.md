# UI 기반 전환 유틸리티

> 36 nodes · cohesion 0.06

## Key Concepts

- **MVActivatableWidgetBase.cpp** (12 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UWidget** (9 connections) — `Source/Maverick/UI/Base/MVUIFadeController.h`
- **MVUIFadeController.cpp** (5 connections) — `Source/Maverick/UI/Base/MVUIFadeController.cpp`
- **UMVActivatableWidgetBase::GetDesiredInputConfig()** (3 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **FMVUIFadeController::Play()** (3 connections) — `Source/Maverick/UI/Base/MVUIFadeController.cpp`
- **MVUIFadeController.h** (3 connections) — `Source/Maverick/UI/Base/MVUIFadeController.h`
- **MVUINavigationLibrary.cpp** (3 connections) — `Source/Maverick/UI/System/MVUINavigationLibrary.cpp`
- **UMVUINavigationLibrary::SetExplicitNavigation()** (3 connections) — `Source/Maverick/UI/System/MVUINavigationLibrary.cpp`
- **UMVUINavigationLibrary::SetNavigationStop()** (3 connections) — `Source/Maverick/UI/System/MVUINavigationLibrary.cpp`
- **MVSkillTreeWindow.cpp** (3 connections) — `Source/Maverick/UI/Window/MVSkillTreeWindow.cpp`
- **EUINavigation** (2 connections)
- **UMVActivatableWidgetBase::NativeGetDesiredFocusTarget()** (2 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::UMVActivatableWidgetBase()** (2 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVUINavigationLibrary::SetCompassNavigation()** (2 connections) — `Source/Maverick/UI/System/MVUINavigationLibrary.cpp`
- **UMVSkillTreeWindow::NativeGetDesiredFocusTarget()** (2 connections) — `Source/Maverick/UI/Window/MVSkillTreeWindow.cpp`
- **FOnFadeFinished** (1 connections)
- **FUIInputConfig** (1 connections)
- **FObjectInitializer** (1 connections)
- **UMVActivatableWidgetBase::DeactivateWidgetWithFade()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::HandleFadeInFinished()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::HandleFadeOutFinished()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::NativeOnActivated()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::NativeOnDeactivated()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::NativeOnHandleBackAction()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- **UMVActivatableWidgetBase::PlayFadeIn()** (1 connections) — `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- *... and 11 more nodes in this community*

## Relationships

- [UI 레이어와 사망 오버레이](UI_%EB%A0%88%EC%9D%B4%EC%96%B4%EC%99%80_%EC%82%AC%EB%A7%9D_%EC%98%A4%EB%B2%84%EB%A0%88%EC%9D%B4.md) (1 shared connections)
- [로딩 창 레이아웃·진행률](%EB%A1%9C%EB%94%A9_%EC%B0%BD_%EB%A0%88%EC%9D%B4%EC%95%84%EC%9B%83%C2%B7%EC%A7%84%ED%96%89%EB%A5%A0.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/Base/MVActivatableWidgetBase.cpp`
- `Source/Maverick/UI/Base/MVUIFadeController.cpp`
- `Source/Maverick/UI/Base/MVUIFadeController.h`
- `Source/Maverick/UI/System/MVUINavigationLibrary.cpp`
- `Source/Maverick/UI/Window/MVSkillTreeWindow.cpp`

## Audit Trail

- EXTRACTED: 78 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*