---
type: community
cohesion: 0.17
members: 12
---

# AMVCharacterBase

**Cohesion:** 0.17 - loosely connected
**Members:** 12 nodes

## Members
- [[AMVCharacterBase()]] - code - Source/Maverick/Character/MVCharacterBase.h
- [[APlayerController_6]] - code
- [[TSubclassOf_2]] - code
- [[UCameraShakeBase_1]] - code
- [[UMVAbilityBaseActiveCameraShake()]] - code - Source/Maverick/Combat/MVAbilityBase.cpp
- [[UMVFieldTransitionSubsystemResetPlayerStatsForTransition()]] - code - Source/Maverick/System/MVFieldTransitionSubsystem.cpp
- [[UMVPIEActionTestWidgetResolveAttackerCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPIEActionTestWidgetResolveTargetCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPIEActionTestWidgetSetTargetCharacter()]] - code - Source/Maverick/UI/Debug/MVPIEActionTestWidget.cpp
- [[UMVPlayerDodgeBeginLockOnPawnRotationSuppressionForDodge()]] - code - Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp
- [[UMVUISubsystemResolvePIEActionTestPlayerController()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp
- [[UMVUISubsystemResolvePIEActionTestTargetCharacter()]] - code - Source/Maverick/UI/System/MVUISubsystem.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/AMVCharacterBase
SORT file.name ASC
```

## Connections to other communities
- 5 edges to [[_COMMUNITY_FVector]]
- 5 edges to [[_COMMUNITY_MVPIEActionTestWidget.cpp]]
- 3 edges to [[_COMMUNITY_MVAbilityBase.cpp]]
- 3 edges to [[_COMMUNITY_MVFieldTransitionSubsystem.cpp]]
- 2 edges to [[_COMMUNITY_MVHitResolverSubsystem.cpp]]
- 2 edges to [[_COMMUNITY_MVUISubsystem.h]]
- 2 edges to [[_COMMUNITY_MVUISubsystem.cpp]]
- 1 edge to [[_COMMUNITY_FindInvincibleCharacter]]
- 1 edge to [[_COMMUNITY_MVCharacterLogAirborneTrace]]
- 1 edge to [[_COMMUNITY_FMVStrafeMoveTaskInstanceData]]
- 1 edge to [[_COMMUNITY_MVWeaponComponent.cpp]]
- 1 edge to [[_COMMUNITY_MVHitReactionLogRecoveryTrace]]
- 1 edge to [[_COMMUNITY_MVDeathRespawnFlow.cpp]]

## Top bridge nodes
- [[AMVCharacterBase()]] - degree 28, connects to 12 communities
- [[UMVAbilityBaseActiveCameraShake()]] - degree 4, connects to 1 community
- [[UMVUISubsystemResolvePIEActionTestPlayerController()]] - degree 3, connects to 1 community
- [[UMVPlayerDodgeBeginLockOnPawnRotationSuppressionForDodge()]] - degree 2, connects to 1 community
- [[UMVFieldTransitionSubsystemResetPlayerStatsForTransition()]] - degree 2, connects to 1 community