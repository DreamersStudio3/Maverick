---
type: community
cohesion: 0.12
members: 17
---

# 적 회피 토큰 지급

**Cohesion:** 0.12 - loosely connected
**Members:** 17 nodes

## Members
- [[MVEnemyDodgeTokenComponent.cpp]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[NotifyEnemyDamaged]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[NotifyEnemyLandedHit]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentBindObservedTargetDamage()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentBindOwnerEvents()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentCanSpendDodgeToken()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentGrantDodgeToken()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentHandleObservedTargetDamaged()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentHandleOwnerDamaged()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentHandleOwnerGroggyStarted()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentIsRelevantEnemyHitData()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentIsRelevantLandedHitData()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentNotifyGroggyEndedByFinisher()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentResetCombatFlowCounters()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentUMVEnemyDodgeTokenComponent()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentUnbindObservedTargetDamage()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentUnbindOwnerEvents()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 4 edges to [[_COMMUNITY_피격 방향과 Launch 처리]]
- 3 edges to [[_COMMUNITY_적 회피 토큰 부여]]
- 3 edges to [[_COMMUNITY_적 회피 토큰 상태]]
- 2 edges to [[_COMMUNITY_컴포넌트 이벤트 바인딩 수명주기]]
- 2 edges to [[_COMMUNITY_회피 토큰 소모와 초기화]]
- 1 edge to [[_COMMUNITY_위협 기반 회피 토큰]]

## Top bridge nodes
- [[MVEnemyDodgeTokenComponent.cpp]] - degree 22, connects to 4 communities
- [[UMVEnemyDodgeTokenComponentHandleObservedTargetDamaged()]] - degree 3, connects to 1 community
- [[UMVEnemyDodgeTokenComponentHandleOwnerDamaged()]] - degree 3, connects to 1 community
- [[NotifyEnemyDamaged]] - degree 2, connects to 1 community
- [[NotifyEnemyLandedHit]] - degree 2, connects to 1 community