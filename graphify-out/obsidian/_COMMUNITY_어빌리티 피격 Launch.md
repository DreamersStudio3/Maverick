---
type: community
cohesion: 0.11
members: 21
---

# 어빌리티 피격 Launch

**Cohesion:** 0.11 - loosely connected
**Members:** 21 nodes

## Members
- [[FMVHitLaunchData()]] - code - Source/Maverick/Public/Struct/MVHitTypes.h
- [[MVAbilityBase.cpp]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[MVAbilityLogHitLaunchTrace()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[TCHAR_4]] - code
- [[TSubclassOf_2]] - code
- [[UActorComponent]] - code - Source/Maverick/Combat/MVAbilityBase.h
- [[UCameraShakeBase_1]] - code
- [[UMVAbilityBaseActiveCameraShake()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseActiveHitStopToCharacters()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseApplyHitLaunchDataToResolveRequest()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseEndAbility_Implementation()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseGetHitLaunchData_Implementation()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseGetOwner()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseGetOwnerCharacter()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseInitAbility()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBasePrepareAbilityExecution()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseSetOwner()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseStartAbility_Implementation()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVAbilityBaseTryConsumeAbilityCost()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UObject_4]] - code
- [[int32_13]] - code

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/__Launch
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_캐릭터 도메인 연동]]
- 1 edge to [[_COMMUNITY_스킬 데이터와 비용]]
- 1 edge to [[_COMMUNITY_피격 판정 해석 파이프라인]]
- 1 edge to [[_COMMUNITY_어빌리티 기본 계약]]
- 1 edge to [[_COMMUNITY_피격 시스템 공용 타입]]

## Top bridge nodes
- [[UMVAbilityBaseActiveCameraShake()]] - degree 4, connects to 1 community
- [[UActorComponent]] - degree 3, connects to 1 community
- [[UMVAbilityBaseApplyHitLaunchDataToResolveRequest()]] - degree 3, connects to 1 community
- [[FMVHitLaunchData()]] - degree 3, connects to 1 community
- [[UMVAbilityBaseActiveHitStopToCharacters()]] - degree 2, connects to 1 community