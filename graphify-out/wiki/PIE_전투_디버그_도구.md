# PIE 전투 디버그 도구

> 15 nodes · cohesion 0.14

## Key Concepts

- **AMVCharacterBase()** (28 connections) — `Source/Maverick/Character/MVCharacterBase.h`
- **UMVAbilityBase::ActiveCameraShake()** (4 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVFieldTransitionSubsystem::ResolvePlayerCharacter()** (3 connections) — `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- **UMVPIEActionTestWidget** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.h`
- **UMVUISubsystem::OpenPIEActionTestPanel()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::ResolvePIEActionTestPlayerController()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVUISubsystem::ShowPIEActionTestPanel()** (3 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **UMVPlayerDodge::BeginLockOnPawnRotationSuppressionForDodge()** (2 connections) — `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp`
- **UMVAbilityBase::ActiveHitStopToCharacters()** (2 connections) — `Source/Maverick/Combat/MVAbilityBase.cpp`
- **UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition()** (2 connections) — `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- **UMVUISubsystem::ResolvePIEActionTestTargetCharacter()** (2 connections) — `Source/Maverick/UI/System/MVUISubsystem.cpp`
- **TSubclassOf** (1 connections)
- **UWorld** (1 connections)
- **APlayerController** (1 connections)
- **UCameraShakeBase** (1 connections)

## Relationships

- [플레이어 회피 방향 해석](%ED%94%8C%EB%A0%88%EC%9D%B4%EC%96%B4_%ED%9A%8C%ED%94%BC_%EB%B0%A9%ED%96%A5_%ED%95%B4%EC%84%9D.md) (5 shared connections)
- [PIE 피격 판정 테스트](PIE_%ED%94%BC%EA%B2%A9_%ED%8C%90%EC%A0%95_%ED%85%8C%EC%8A%A4%ED%8A%B8.md) (5 shared connections)
- [UI 서브시스템 팝업 관리](UI_%EC%84%9C%EB%B8%8C%EC%8B%9C%EC%8A%A4%ED%85%9C_%ED%8C%9D%EC%97%85_%EA%B4%80%EB%A6%AC.md) (4 shared connections)
- [어빌리티 피격 Launch](%EC%96%B4%EB%B9%8C%EB%A6%AC%ED%8B%B0_%ED%94%BC%EA%B2%A9_Launch.md) (3 shared connections)
- [필드 전환 관리](%ED%95%84%EB%93%9C_%EC%A0%84%ED%99%98_%EA%B4%80%EB%A6%AC.md) (3 shared connections)
- [피격 판정 해석 파이프라인](%ED%94%BC%EA%B2%A9_%ED%8C%90%EC%A0%95_%ED%95%B4%EC%84%9D_%ED%8C%8C%EC%9D%B4%ED%94%84%EB%9D%BC%EC%9D%B8.md) (2 shared connections)
- [무적 구간 노티파이 상태](%EB%AC%B4%EC%A0%81_%EA%B5%AC%EA%B0%84_%EB%85%B8%ED%8B%B0%ED%8C%8C%EC%9D%B4_%EC%83%81%ED%83%9C.md) (1 shared connections)
- [캐릭터 공중 피격 추적](%EC%BA%90%EB%A6%AD%ED%84%B0_%EA%B3%B5%EC%A4%91_%ED%94%BC%EA%B2%A9_%EC%B6%94%EC%A0%81.md) (1 shared connections)
- [AI 스트레이프 이동](AI_%EC%8A%A4%ED%8A%B8%EB%A0%88%EC%9D%B4%ED%94%84_%EC%9D%B4%EB%8F%99.md) (1 shared connections)
- [무기 장착 시각화](%EB%AC%B4%EA%B8%B0_%EC%9E%A5%EC%B0%A9_%EC%8B%9C%EA%B0%81%ED%99%94.md) (1 shared connections)
- [피격 회복 방향 추적](%ED%94%BC%EA%B2%A9_%ED%9A%8C%EB%B3%B5_%EB%B0%A9%ED%96%A5_%EC%B6%94%EC%A0%81.md) (1 shared connections)
- [사망 부활 오케스트레이션](%EC%82%AC%EB%A7%9D_%EB%B6%80%ED%99%9C_%EC%98%A4%EC%BC%80%EC%8A%A4%ED%8A%B8%EB%A0%88%EC%9D%B4%EC%85%98.md) (1 shared connections)

## Source Files

- `Source/Maverick/Character/MVCharacterBase.h`
- `Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp`
- `Source/Maverick/Combat/MVAbilityBase.cpp`
- `Source/Maverick/System/MVFieldTransitionSubsystem.cpp`
- `Source/Maverick/UI/System/MVUISubsystem.cpp`
- `Source/Maverick/UI/System/MVUISubsystem.h`

## Audit Trail

- EXTRACTED: 59 (100%)
- INFERRED: 0 (0%)
- AMBIGUOUS: 0 (0%)

---

*Part of the graphify knowledge wiki. See [index](index.md) to navigate.*