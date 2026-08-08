---
type: community
members: 4
---

# ResetCombatFlowCounters

**Members:** 4 nodes

## Members
- [[CanSpendDodgeToken]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[ResetCombatFlowCounters]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.h
- [[UMVEnemyDodgeTokenComponentResetForFieldTransition()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp
- [[UMVEnemyDodgeTokenComponentTrySpendDodgeToken()]] - code - Source/Maverick/Components/MVEnemyDodgeTokenComponent.cpp

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/ResetCombatFlowCounters
SORT file.name ASC
```

## Connections to other communities
- 2 edges to [[_COMMUNITY_MVEnemyDodgeTokenComponent.cpp]]
- 2 edges to [[_COMMUNITY_EMVEnemyDodgeTokenGrantReason]]

## Top bridge nodes
- [[UMVEnemyDodgeTokenComponentTrySpendDodgeToken()]] - degree 3, connects to 1 community
- [[ResetCombatFlowCounters]] - degree 3, connects to 1 community
- [[UMVEnemyDodgeTokenComponentResetForFieldTransition()]] - degree 2, connects to 1 community
- [[CanSpendDodgeToken]] - degree 2, connects to 1 community