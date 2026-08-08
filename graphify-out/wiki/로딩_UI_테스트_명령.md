# 로딩 UI 테스트 명령

> 14 nodes · cohesion 0.25

## Key Concepts

- **UWorld** (7 connections)
- **MVUISubsystemResolveLoadingTestSubsystem()** (6 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **MVUISubsystemShowLoadingTestCommand()** (6 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **MVUISubsystemAdvanceLoadingTestCardCommand()** (5 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **MVUISubsystemHideLoadingTestCommand()** (5 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **FString** (4 connections)
- **TArray** (4 connections)
- **MVUISubsystemShouldUseNativeLoadingTestWindow()** (4 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUILayerBase()** (3 connections) — `Source/Maverick/UI/System/MVUILayerBase.h`
- **UMVUISubsystem::GetOrCreateRootLayer()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::HandleWorldInit()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::PushLayer()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **InitializationValues** (1 connections)
- **UMVUISubsystem** (1 connections)

## Relationships

- [UI 서브시스템 팝업 관리](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%ED%8C%9D%EC%97%85_%EA%B4%80%EB%A6%AC.md) (8 shared connections)
- [UI 레이어와 사망 오버레이](UI_%EB%A0%88%EC%9D%B4%EC%96%B4%EC%99%80_%EC%82%AC%EB%A7%9D_%EC%98%A4%EB%B2%84%EB%A0%88%EC%9D%B4.md) (1 shared connections)

## Source Files

- `Source/Maverick/UI/System/MVUILayerBase.h`
- `Source/Maverick/UI/System/MVUISubsystem.cpp`

## Audit Trail

- EXTRACTED: 55 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*