# 어빌리티 피격 Launch

> 28 nodes

## Key Concepts

- **AMVCharacterBase()** (28 connections) — `Source/Maverick/Character/MVCharacterBase.h`
- **MVAbilityBase.cpp** (13 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **MVAbilityLogHitLaunchTrace()** (6 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::ActiveCameraShake()** (4 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::GetHitLaunchData_Implementation()** (3 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::ApplyHitLaunchDataToResolveRequest()** (3 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UActorComponent** (3 connections) — `Source/Maverick/Combat/MVAbilityBase.h`
- **FMVHitLaunchData()** (3 connections) — `Source/Maverick/Public/Struct/MVHitTypes.h`
- **UMVPlayerDodge::BeginLockOnPawnRotationSuppressionForDodge()** (2 connections) — `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp`
- **UMVAbilityBase::SetOwner()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::GetOwner()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::GetOwnerCharacter()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::InitAbility()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::StartAbility_Implementation()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVAbilityBase::ActiveHitStopToCharacters()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition()** (2 connections) — `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- **UMVPIEActionTestWidget::SetTargetCharacter()** (2 connections) — `Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp`
- **UMVPIEActionTestWidget::ResolveTargetCharacter()** (2 connections) — `Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp`
- **UMVPIEActionTestWidget::ResolveAttackerCharacter()** (2 connections) — `Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp`
- **UMVUISubsystem::ResolvePIEActionTestTargetCharacter()** (2 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UObject** (1 connections)
- **TCHAR** (1 connections)
- **UMVAbilityBase::PrepareAbilityExecution()** (1 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **int32** (1 connections)
- **UMVAbilityBase::EndAbility_Implementation()** (1 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- *... and 3 more nodes in this community*

## Relationships

- [회피 입력 방향 구성](%ED%9A%8C%ED%94%BC_%EC%9E%85%EB%A0%A5_%EB%B0%A9%ED%96%A5_%EA%B5%AC%EC%84%B1.md) (4 shared connections)
- [PIE 피격 판정 테스트](PIE_%ED%94%BC%EA%B2%A9_%ED%8C%90%EC%A0%95_%ED%85%8C%EC%8A%A4%ED%8A%B8.md) (4 shared connections)
- [피격 판정 해석 파이프라인](%ED%94%BC%EA%B2%A9_%ED%8C%90%EC%A0%95_%ED%95%B4%EC%84%9D_%ED%8C%8C%EC%9D%B4%ED%94%84%EB%9D%BC%EC%9D%B8.md) (3 shared connections)
- [필드 전환 관리](%ED%95%84%EB%93%9C_%EC%A0%84%ED%99%98_%EA%B4%80%EB%A6%AC.md) (3 shared connections)
- [피격 시스템 공용 타입](%ED%94%BC%EA%B2%A9_%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%ED%83%80%EC%9E%85.md) (3 shared connections)
- [UI 서브시스템 공용 선언](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%EA%B3%B5%EC%9A%A9_%EC%84%A0%EC%96%B8.md) (2 shared connections)
- [UI 서브시스템 팝업 관리](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%ED%8C%9D%EC%97%85_%EA%B4%80%EB%A6%AC.md) (2 shared connections)
- [무적 구간 노티파이 상태](%EB%AC%B4%EC%A0%81_%EA%B5%AC%EA%B0%84_%EB%85%B8%ED%8B%B0%ED%8C%8C%EC%9D%B4_%EC%83%81%ED%83%9C.md) (1 shared connections)
- [피격 공중 상태 추적](%ED%94%BC%EA%B2%A9_%EA%B3%B5%EC%A4%91_%EC%83%81%ED%83%9C_%EC%B6%94%EC%A0%81.md) (1 shared connections)
- [공통 캐릭터 이동 선언](%EA%B3%B5%ED%86%B5_%EC%BA%90%EB%A6%AD%ED%84%B0_%EC%9D%B4%EB%8F%99_%EC%84%A0%EC%96%B8.md) (1 shared connections)
- [플레이어 회피 방향 해석](%ED%94%8C%EB%A0%88%EC%9D%B4%EC%96%B4_%ED%9A%8C%ED%94%BC_%EB%B0%A9%ED%96%A5_%ED%95%B4%EC%84%9D.md) (1 shared connections)
- [무기 장착 시각화](%EB%AC%B4%EA%B8%B0_%EC%9E%A5%EC%B0%A9_%EC%8B%9C%EA%B0%81%ED%99%94.md) (1 shared connections)

## Source Files

- `Source/Maverick/Character/MVCharacterBase.h`
- `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp`
- `Source/Maverick/Combat/MVAbilityBase.cpp`
- `Source/Maverick/Combat/MVAbilityBase.h`
- `Source/Maverick/Public/Struct/MVHitTypes.h`
- `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- `Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp`
- `Source/Maverick/UI/System/MVUISubsystem.cpp`

## Audit Trail

- EXTRACTED: 95 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*