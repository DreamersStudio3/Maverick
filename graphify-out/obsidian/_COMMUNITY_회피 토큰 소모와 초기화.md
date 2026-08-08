---
type: community
members: 4
---

# 회피 토큰 소모와 초기화

**Members:** 4 nodes

## Members
- [[CanSpendDodgeToken]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[ResetCombatFlowCounters]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentResetForFieldTransition()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentTrySpendDodgeToken()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/___
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_적 회피 토큰 지급]]
- 2 edges to [[_COMMUNITY_적 회피 토큰 상태]]

## Top bridge nodes
- [[UMVEnemyDodgeTokenComponentTrySpendDodgeToken()]] - degree 3, connects to 1 community
- [[ResetCombatFlowCounters]] - degree 3, connects to 1 community
- [[UMVEnemyDodgeTokenComponentResetForFieldTransition()]] - degree 2, connects to 1 community
- [[CanSpendDodgeToken]] - degree 2, connects to 1 community