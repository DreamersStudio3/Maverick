---
type: community
cohesion: 0.14
members: 15
---

# PIE 전투 디버그 도구

**Cohesion:** 0.14 - loosely connected
**Members:** 15 nodes

## Members
- [[AMVCharacterBase()]] - code - Source/Maverick/Character/MVCharacterBase.h
- [[APlayerController_6]] - code
- [[TSubclassOf_2]] - code
- [[UCameraShakeBase_1]] - code
- [[UMVAbilityBaseActiveCameraShake()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseActiveHitStopToCharacters()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVFieldTransitionSubsystemResetPlayerStatsForTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVFieldTransitionSubsystemResolvePlayerCharacter()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVPIEActionTestWidget]] - code - Source/Maverick/UI/System/MVUISubsystem.h
- [[UMVPlayerDodgeBeginLockOnPawnRotationSuppressionForDodge()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVUISubsystemOpenPIEActionTestPanel()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemResolvePIEActionTestPlayerController()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemResolvePIEActionTestTargetCharacter()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemShowPIEActionTestPanel()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UWorld_12]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/PIE___
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_플레이어 회피 방향 해석]]
- 5 edges to [[_COMMUNITY_PIE 피격 판정 테스트]]
- 4 edges to [[_COMMUNITY_UI 서브시스템 팝업 관리]]
- 3 edges to [[_COMMUNITY_어빌리티 피격 Launch]]
- 3 edges to [[_COMMUNITY_필드 전환 관리]]
- 2 edges to [[_COMMUNITY_피격 판정 해석 파이프라인]]
- 1 edge to [[_COMMUNITY_무적 구간 노티파이 상태]]
- 1 edge to [[_COMMUNITY_캐릭터 공중 피격 추적]]
- 1 edge to [[_COMMUNITY_AI 스트레이프 이동]]
- 1 edge to [[_COMMUNITY_무기 장착 시각화]]
- 1 edge to [[_COMMUNITY_피격 회복 방향 추적]]
- 1 edge to [[_COMMUNITY_사망 부활 오케스트레이션]]
- 1 edge to [[_COMMUNITY_UI 서브시스템 공용 선언]]

## Top bridge nodes
- [[AMVCharacterBase()]] - degree 28, connects to 11 communities
- [[UMVAbilityBaseActiveCameraShake()]] - degree 4, connects to 1 community
- [[UMVFieldTransitionSubsystemResolvePlayerCharacter()]] - degree 3, connects to 1 community
- [[UMVPIEActionTestWidget]] - degree 3, connects to 1 community
- [[UMVUISubsystemOpenPIEActionTestPanel()]] - degree 3, connects to 1 community