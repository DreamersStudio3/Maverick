---
type: community
cohesion: 0.14
members: 14
---

# 필드 전환 리셋 계약

**Cohesion:** 0.14 - loosely connected
**Members:** 14 nodes

## Members
- [[AMVEnemyHandleFieldTransitionReset_Implementation()]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.cpp
- [[FMVFieldTransitionResetContext()]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[IMVFieldTransitionResettableInterface()]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[MAVERICK_API]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[MAVERICK_API_13]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[MVEnemy.h]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[MVFieldTransitionResettableInterface.h]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[MVUISettings.cpp]] - code - Source/Maverick/UI/System/MVUISettings.cpp
- [[UAnimMontage_2]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVDeathOverlayWindow]] - code - Source/Maverick/System/MVDeathRespawnFlow.h
- [[UMVEnemyDodgeTokenComponent]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVMainHUDWidget]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVUISettingsUMVUISettings()]] - code - Source/Maverick/UI/System/MVUISettings.cpp
- [[UMVWorldStateSubsystem]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_전역 전투 센싱 태스크]]
- 1 edge to [[_COMMUNITY_적 캐릭터 전투 실행]]
- 1 edge to [[_COMMUNITY_피격 리액션 회복 결정]]
- 1 edge to [[_COMMUNITY_사망 부활 오케스트레이션]]
- 1 edge to [[_COMMUNITY_필드 전환 관리]]
- 1 edge to [[_COMMUNITY_상호작용 프롬프트 UI]]

## Top bridge nodes
- [[MVEnemy.h]] - degree 7, connects to 2 communities
- [[MVFieldTransitionResettableInterface.h]] - degree 6, connects to 1 community
- [[UMVUISettingsUMVUISettings()]] - degree 4, connects to 1 community
- [[AMVEnemyHandleFieldTransitionReset_Implementation()]] - degree 2, connects to 1 community
- [[UMVDeathOverlayWindow]] - degree 2, connects to 1 community