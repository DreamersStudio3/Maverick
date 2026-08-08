---
type: community
cohesion: 0.20
members: 10
---

# FMVCombatActionEvent

**Cohesion:** 0.20 - loosely connected
**Members:** 10 nodes

## Members
- [[ActiveStackIndex]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChainWindowDuration]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[ChainWindowRemaining]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CooldownDuration]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[CooldownRemaining]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[FMVSkillSlotRuntimeState]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[StackSize]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bAvailable]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bChainActive_1]] - code - Source/Maverick/Components/MVCombatComponent.h
- [[bOnCooldown]] - code - Source/Maverick/Components/MVCombatComponent.h

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/FMVCombatActionEvent
SORT file.name ASC
```

## Connections to other communities
- 1 edge to [[_COMMUNITY_int32]]
- 1 edge to [[_COMMUNITY_FMVSkillEntry]]
- 1 edge to [[_COMMUNITY_UMVDodgeThreatEvaluatorLibraryEvaluateDodgeThreat]]

## Top bridge nodes
- [[FMVSkillSlotRuntimeState]] - degree 12, connects to 3 communities