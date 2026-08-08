---
type: community
cohesion: 0.20
members: 10
---

# 필드 전환 리셋 계약

**Cohesion:** 0.20 - loosely connected
**Members:** 10 nodes

## Members
- [[AMVEnemyHandleFieldTransitionReset_Implementation()]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.cpp
- [[FMVFieldTransitionResetContext()]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[IMVFieldTransitionResettableInterface()]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[MAVERICK_API]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[MAVERICK_API_13]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[MVEnemy.h]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[MVFieldTransitionResettableInterface.h]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h
- [[UAnimMontage_2]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVEnemyDodgeTokenComponent]] - code - Source/Maverick/Character/NPC/Enemy/MVEnemy.h
- [[UMVWorldStateSubsystem]] - code - Source/Maverick/System/MVFieldTransitionResettableInterface.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_AI 기본 공격 태스크]]
- 1 edge to [[_COMMUNITY_적 캐릭터 전투 실행]]
- 1 edge to [[_COMMUNITY_사망 부활 UI 흐름]]
- 1 edge to [[_COMMUNITY_피격 리액션 회복 결정]]
- 1 edge to [[_COMMUNITY_필드 전환 관리]]

## Top bridge nodes
- [[MVEnemy.h]] - degree 7, connects to 3 communities
- [[MVFieldTransitionResettableInterface.h]] - degree 6, connects to 1 community
- [[AMVEnemyHandleFieldTransitionReset_Implementation()]] - degree 2, connects to 1 community