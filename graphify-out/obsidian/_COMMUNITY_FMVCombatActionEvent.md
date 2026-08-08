---
type: community
cohesion: 0.20
members: 10
---

# FMVCombatActionEvent

**Cohesion:** 0.20 - loosely connected
**Members:** 10 nodes

## Members
- [[ChargeActionMapKey]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChargeActionTableName]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChargeChainStageIndex]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChargeRowHandle]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CommitTime]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[FMVCombatHeavyChargeAttackRuntimeState]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[StartedWorldTime]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[UMVCombatComponentResetHeavyChargeAttackState()]] - code - Source/Maverick/Components/MVCombatComponent.cpp
- [[bActive]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bCommitted]] - code - Source/Maverick/Components/MVCombatComponent.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/FMVCombatActionEvent
SORT file.name ASC
```

## Connections to other communities
- 3 edges to [[_COMMUNITY_FMVSkillEntry]]
- 2 edges to [[_COMMUNITY_MVCombatComponent.cpp]]
- 1 edge to [[_COMMUNITY_UMVDodgeThreatEvaluatorLibraryEvaluateDodgeThreat]]

## Top bridge nodes
- [[FMVCombatHeavyChargeAttackRuntimeState]] - degree 14, connects to 3 communities
- [[UMVCombatComponentResetHeavyChargeAttackState()]] - degree 2, connects to 1 community