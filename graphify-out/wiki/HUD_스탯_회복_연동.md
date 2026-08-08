# HUD 스탯 회복 연동

> 44 nodes · cohesion 0.05

## Key Concepts

- **MVBossHPBarWidget.cpp** (13 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **MVPlayerStatusWidget.cpp** (12 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **FindPauseRecoverableStatRecoveryStatComponent()** (5 connections) — `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- **UMVAnimNotifyState_PauseRecoverableStatRecovery::NotifyBegin()** (5 connections) — `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- **UMVAnimNotifyState_PauseRecoverableStatRecovery::NotifyEnd()** (5 connections) — `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- **MVStatComponent.h** (5 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **UMVStatComponent()** (5 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **MVAnimNotifyState_PauseRecoverableStatRecovery.cpp** (4 connections) — `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- **USkeletalMeshComponent** (3 connections)
- **UMVBossHPBarWidget::BindToStatComponent()** (3 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **MVStatusBarWidget.h** (3 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`
- **FAnimNotifyEventReference** (2 connections)
- **UAnimSequenceBase** (2 connections)
- **UMVAnimNotifyState_PauseRecoverableStatRecovery::GetNotifyName_Implementation()** (2 connections) — `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- **FText** (2 connections)
- **UMVBossHPBarWidget::HandleDamageAccumulated()** (2 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **UMVBossHPBarWidget::InitializeBossBar()** (2 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **UMVPlayerStatusWidget::ApplyStatusBarSize()** (2 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVPlayerStatusWidget::BindToStatComponent()** (2 connections) — `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- **UMVStatusBarWidget()** (2 connections) — `Source/Maverick/UI/HUD/MVStatusBarWidget.h`
- **FString** (1 connections)
- **AActor** (1 connections) — `Source/Maverick/Components/MVStatComponent.h`
- **UMVBossHPBarWidget::HandleDamageAccumulationReset()** (1 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **UMVBossHPBarWidget::HandleGroggyChanged()** (1 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- **UMVBossHPBarWidget::HandleGroggyEnded()** (1 connections) — `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- *... and 19 more nodes in this community*

## Relationships

- [피격 시스템 공용 타입](%ED%94%BC%EA%B2%A9_%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%ED%83%80%EC%9E%85.md) (1 shared connections)
- [사망 표현 보류·시작](%EC%82%AC%EB%A7%9D_%ED%91%9C%ED%98%84_%EB%B3%B4%EB%A5%98%C2%B7%EC%8B%9C%EC%9E%91.md) (1 shared connections)
- [테이블 기반 UI·스탯 조회](%ED%85%8C%EC%9D%B4%EB%B8%94_%EA%B8%B0%EB%B0%98_UI%C2%B7%EC%8A%A4%ED%83%AF_%EC%A1%B0%ED%9A%8C.md) (1 shared connections)
- [메인 HUD 상태 바인딩](%EB%A9%94%EC%9D%B8_HUD_%EC%83%81%ED%83%9C_%EB%B0%94%EC%9D%B8%EB%94%A9.md) (1 shared connections)
- [피격 방향과 Launch 처리](%ED%94%BC%EA%B2%A9_%EB%B0%A9%ED%96%A5%EA%B3%BC_Launch_%EC%B2%98%EB%A6%AC.md) (1 shared connections)

## Source Files

- `Source/Maverick/Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.cpp`
- `Source/Maverick/Components/MVStatComponent.h`
- `Source/Maverick/UI/HUD/MVBossHPBarWidget.cpp`
- `Source/Maverick/UI/HUD/MVPlayerStatusWidget.cpp`
- `Source/Maverick/UI/HUD/MVStatusBarWidget.h`

## Audit Trail

- EXTRACTED: 105 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*